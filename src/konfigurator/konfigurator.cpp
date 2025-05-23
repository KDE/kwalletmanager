/*
    SPDX-FileCopyrightText: 2003 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "konfigurator.h"
#include "../kwalletmanager_version.h"

#include <KAuth/Action>
#include <KAuth/ActionReply>
#include <KAuth/ExecuteJob>
#include <KConfigGroup>
#include <KMessageBox>
#include <KPluginFactory>
#include <KWallet>
#include <QInputDialog>

#include <KAboutData>

#include <QCheckBox>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDebug>
#include <QFile>
#include <QMenu>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>

#define KWALLETMANAGERINTERFACE "org.kde.KWallet"

K_PLUGIN_CLASS_WITH_JSON(KWalletConfig, "kwalletconfig.json")

KWalletConfig::KWalletConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , _wcw(new WalletConfigWidget(widget()))
    , _cfg(KSharedConfig::openConfig(QStringLiteral("kwalletrc"), KConfig::NoGlobals))
{
    setAuthActionName(QStringLiteral("org.kde.kcontrol.kcmkwallet5.save"));
    auto vbox = new QVBoxLayout(widget());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(_wcw);

    connect(_wcw->_enabled, &QCheckBox::clicked, this, &KWalletConfig::configChanged);
    connect(_wcw->_launchManager, &QCheckBox::clicked, this, &KWalletConfig::configChanged);
    connect(_wcw->_autocloseManager, &QCheckBox::clicked, this, &KWalletConfig::configChanged);
    connect(_wcw->_autoclose, &QCheckBox::clicked, this, &KWalletConfig::configChanged);
    connect(_wcw->_closeIdle, &QCheckBox::clicked, this, &KWalletConfig::configChanged);
    connect(_wcw->_openPrompt, &QCheckBox::clicked, this, &KWalletConfig::configChanged);
    connect(_wcw->_screensaverLock, &QCheckBox::clicked, this, &KWalletConfig::configChanged);
    connect(_wcw->_localWalletSelected, &QCheckBox::clicked, this, &KWalletConfig::configChanged);
    connect(_wcw->_idleTime, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &KWalletConfig::configChanged);
    connect(_wcw->_launch, &QPushButton::clicked, this, &KWalletConfig::launchManager);
    connect(_wcw->_newWallet, &QPushButton::clicked, this, &KWalletConfig::newNetworkWallet);
    connect(_wcw->_newLocalWallet, &QPushButton::clicked, this, &KWalletConfig::newLocalWallet);
    connect(_wcw->_localWallet, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &KWalletConfig::configChanged);
    connect(_wcw->_defaultWallet, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &KWalletConfig::configChanged);
    connect(_wcw->_accessList, &QTreeWidget::customContextMenuRequested, this, &KWalletConfig::customContextMenuRequested);
    connect(_wcw->_secretServiceAPI, &QCheckBox::clicked, this, &KWalletConfig::configChanged);

    QStyle *style = widget()->style();
    _wcw->launchButtonBar->setContentsMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                              0,
                                              style->pixelMetric(QStyle::PM_LayoutRightMargin),
                                              style->pixelMetric(QStyle::PM_LayoutBottomMargin));

    _wcw->_accessList->setAllColumnsShowFocus(true);
    _wcw->_accessList->setContextMenuPolicy(Qt::CustomContextMenu);
    _wcw->tabWidget2->tabBar()->setExpanding(true);
    updateWalletLists();

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.kwalletmanager"))) {
        _wcw->_launch->hide();
    }
}

KWalletConfig::~KWalletConfig() = default;

void KWalletConfig::updateWalletLists()
{
    const QString p1(_wcw->_localWallet->currentText());
    const QString p2(_wcw->_defaultWallet->currentText());

    _wcw->_localWallet->clear();
    _wcw->_defaultWallet->clear();

    const QStringList wl = KWallet::Wallet::walletList();
    _wcw->_localWallet->addItems(wl);
    _wcw->_defaultWallet->addItems(wl);

    int index = wl.indexOf(p1);
    if (index != -1) {
        _wcw->_localWallet->setCurrentIndex(index);
    }

    index = wl.indexOf(p2);
    if (index != -1) {
        _wcw->_defaultWallet->setCurrentIndex(index);
    }
}

QString KWalletConfig::newWallet()
{
    bool ok;
    const QString n = QInputDialog::getText(widget(), i18n("New Wallet"), i18n("Please choose a name for the new wallet:"), QLineEdit::Normal, QString(), &ok);
    if (!ok) {
        return {};
    }
    KWallet::Wallet *w = KWallet::Wallet::openWallet(n, widget()->topLevelWidget()->winId());
    if (!w) {
        return {};
    }

    delete w;
    return n;
}

void KWalletConfig::newLocalWallet()
{
    const QString n = newWallet();
    if (n.trimmed().isEmpty()) {
        return;
    }

    updateWalletLists();

    _wcw->_localWallet->setCurrentIndex(_wcw->_localWallet->findText(n));
    setNeedsSave(true);
}

void KWalletConfig::newNetworkWallet()
{
    const QString n = newWallet();
    if (n.trimmed().isEmpty()) {
        return;
    }

    updateWalletLists();

    _wcw->_defaultWallet->setCurrentIndex(_wcw->_defaultWallet->findText(n));
    setNeedsSave(true);
}

void KWalletConfig::launchManager()
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.kwalletmanager5"))) {
        QProcess::startDetached(QStringLiteral("kwalletmanager5"), QStringList(QStringLiteral("--show")));
    } else {
        QDBusInterface kwalletd(QStringLiteral("org.kde.kwalletmanager5"), QStringLiteral("/kwalletmanager5/MainWindow_1"));
        kwalletd.call(QStringLiteral("show"));
        kwalletd.call(QStringLiteral("raise"));
    }
}

void KWalletConfig::configChanged()
{
    setNeedsSave(true);
}

void KWalletConfig::load()
{
    KConfigGroup config(_cfg, QStringLiteral("Wallet"));
    _wcw->_enabled->setChecked(config.readEntry("Enabled", true));
    _wcw->_openPrompt->setChecked(config.readEntry("Prompt on Open", false));
    _wcw->_launchManager->setChecked(config.readEntry("Launch Manager", false));
    _wcw->_autocloseManager->setChecked(!config.readEntry("Leave Manager Open", false));
    _wcw->_screensaverLock->setChecked(config.readEntry("Close on Screensaver", false));
    _wcw->_autoclose->setChecked(!config.readEntry("Leave Open", true));
    _wcw->_closeIdle->setChecked(config.readEntry("Close When Idle", false));
    _wcw->_idleTime->setValue(config.readEntry("Idle Timeout", 10));
    if (config.hasKey("Default Wallet")) {
        int defaultWallet_idx = _wcw->_defaultWallet->findText(config.readEntry("Default Wallet"));
        if (defaultWallet_idx != -1) {
            _wcw->_defaultWallet->setCurrentIndex(defaultWallet_idx);
        } else {
            _wcw->_defaultWallet->setCurrentIndex(0);
        }
    } else {
        _wcw->_defaultWallet->setCurrentIndex(0);
    }
    if (config.hasKey("Local Wallet")) {
        _wcw->_localWalletSelected->setChecked(!config.readEntry("Use One Wallet", false));
        int localWallet_idx = _wcw->_localWallet->findText(config.readEntry("Local Wallet"));
        if (localWallet_idx != -1) {
            _wcw->_localWallet->setCurrentIndex(localWallet_idx);
        } else {
            _wcw->_localWallet->setCurrentIndex(0);
        }
    } else {
        _wcw->_localWalletSelected->setChecked(false);
    }
    _wcw->_accessList->clear();
    KConfigGroup ad(_cfg, QStringLiteral("Auto Deny"));
    KConfigGroup aa(_cfg, QStringLiteral("Auto Allow"));
    QStringList denykeys = ad.entryMap().keys();
    const QStringList keys = aa.entryMap().keys();
    for (QStringList::const_iterator i = keys.begin(); i != keys.end(); ++i) {
        QString walletName = *i;
        // perform cleanup in the kwalletrc file, by removing entries that correspond to non-existent
        // (previously deleted, for example) wallets
        QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        path.append(QStringLiteral("/kwalletd/%1.kwl").arg(walletName));
        if (!QFile::exists(path)) {
            // if the wallet no longer exists, delete the entries from the configuration file and skip to next entry
            KConfigGroup cfgAllow = KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group(QStringLiteral("Auto Allow"));
            cfgAllow.deleteEntry(walletName);

            KConfigGroup cfgDeny = KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group(QStringLiteral("Auto Deny"));
            cfgDeny.deleteEntry(walletName);
            continue;
        }

        const QStringList apps = aa.readEntry(*i, QStringList());
        const QStringList denyapps = ad.readEntry(*i, QStringList());
        denykeys.removeAll(walletName);
        auto twi = new QTreeWidgetItem(_wcw->_accessList, QStringList() << walletName);

        for (QStringList::const_iterator j = apps.begin(), end = apps.end(); j != end; ++j) {
            new QTreeWidgetItem(twi, QStringList() << QString() << *j << i18n("Always Allow"));
        }
        for (QStringList::const_iterator j = denyapps.begin(), end = denyapps.end(); j != end; ++j) {
            new QTreeWidgetItem(twi, QStringList() << QString() << *j << i18n("Always Deny"));
        }
    }
    for (QStringList::const_iterator i = denykeys.constBegin(), denykeysEnd = denykeys.constEnd(); i != denykeysEnd; ++i) {
        const QStringList denyapps = ad.readEntry(*i, QStringList());
        auto twi = new QTreeWidgetItem(_wcw->_accessList, QStringList() << *i);
        for (QStringList::const_iterator j = denyapps.begin(), denyappsEnd = denyapps.end(); j != denyappsEnd; ++j) {
            new QTreeWidgetItem(twi, QStringList() << QString() << *j << i18n("Always Deny"));
        }
    }
    _wcw->_accessList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    KConfigGroup secretsAPIConfig(_cfg, QStringLiteral("org.freedesktop.secrets"));
    _wcw->_secretServiceAPI->setChecked(secretsAPIConfig.readEntry("apiEnabled", true));
    setNeedsSave(false);
}

void KWalletConfig::save()
{
    QVariantMap args;
    KAuth::Action action(QLatin1String("org.kde.kcontrol.kcmkwallet5.save"));
    action.setHelperId(QStringLiteral("org.kde.kcontrol.kcmkwallet5"));

    widget()->window()->winId();
    action.setParentWindow(widget()->window()->windowHandle());
    if (!action.isValid()) {
        qDebug() << "There's no authAction, not saving settings";
        return;
    }
    action.setArguments(args);

    KAuth::ExecuteJob *j = action.execute();

    if (!j->exec()) {
        if (j->error() == KAuth::ActionReply::AuthorizationDeniedError) {
            KMessageBox::error(widget(), i18n("Permission denied."), i18n("KDE Wallet Control Module"));
        } else {
            KMessageBox::error(widget(), i18n("Error while authenticating action:\n%1", j->errorString()), i18n("KDE Wallet Control Module"));
        }
        load();
        return;
    }

    KConfigGroup config(_cfg, QStringLiteral("Wallet"));
    config.writeEntry("Enabled", _wcw->_enabled->isChecked());
    config.writeEntry("Launch Manager", _wcw->_launchManager->isChecked());
    config.writeEntry("Leave Manager Open", !_wcw->_autocloseManager->isChecked());
    config.writeEntry("Leave Open", !_wcw->_autoclose->isChecked());
    config.writeEntry("Close When Idle", _wcw->_closeIdle->isChecked());
    config.writeEntry("Idle Timeout", _wcw->_idleTime->value());
    config.writeEntry("Prompt on Open", _wcw->_openPrompt->isChecked());
    config.writeEntry("Close on Screensaver", _wcw->_screensaverLock->isChecked());

    config.writeEntry("Use One Wallet", !_wcw->_localWalletSelected->isChecked());
    if (_wcw->_localWalletSelected->isChecked()) {
        config.writeEntry("Local Wallet", _wcw->_localWallet->currentText());
    } else {
        config.deleteEntry("Local Wallet");
    }

    if (_wcw->_defaultWallet->currentIndex() != -1) {
        config.writeEntry("Default Wallet", _wcw->_defaultWallet->currentText());
    } else {
        config.deleteEntry("Default Wallet");
    }

    // FIXME: won't survive a language change
    _cfg->deleteGroup(QStringLiteral("Auto Allow"));
    _cfg->deleteGroup(QStringLiteral("Auto Deny"));
    config = _cfg->group(QStringLiteral("Auto Allow"));
    for (int i = 0; i < _wcw->_accessList->topLevelItemCount(); ++i) {
        QTreeWidgetItem *parentItem = _wcw->_accessList->topLevelItem(i);
        QStringList al;
        for (int j = 0; j < parentItem->childCount(); ++j) {
            QTreeWidgetItem *childItem = parentItem->child(j);
            if (childItem->text(2) == i18n("Always Allow")) {
                al << childItem->text(1);
            }
        }
        config.writeEntry(parentItem->text(0), al);
    }

    config = _cfg->group(QStringLiteral("Auto Deny"));
    for (int i = 0; i < _wcw->_accessList->topLevelItemCount(); ++i) {
        QTreeWidgetItem *parentItem = _wcw->_accessList->topLevelItem(i);
        QStringList al;
        for (int j = 0; j < parentItem->childCount(); ++j) {
            QTreeWidgetItem *childItem = parentItem->child(j);
            if (childItem->text(2) == i18n("Always Deny")) {
                al << childItem->text(1);
            }
        }
        config.writeEntry(parentItem->text(0), al);
    }

    KConfigGroup secretsAPIConfig(_cfg, QStringLiteral("org.freedesktop.secrets"));
    secretsAPIConfig.writeEntry("apiEnabled", _wcw->_secretServiceAPI->isChecked());

    _cfg->sync();

    // this restarts kwalletd if necessary
    QDBusInterface kwalletd(QStringLiteral("org.kde.kwalletd5"), QStringLiteral("/modules/kwalletd5"), QStringLiteral(KWALLETMANAGERINTERFACE));
    // if wallet was deactivated, then kwalletd will exit upon start so check
    // the status before invoking reconfigure
    if (kwalletd.isValid()) {
        // this will eventually make kwalletd exit upon deactivation
        kwalletd.call(QStringLiteral("reconfigure"));
    }
    setNeedsSave(false);
}

void KWalletConfig::defaults()
{
    _wcw->_enabled->setChecked(true);
    _wcw->_openPrompt->setChecked(false);
    _wcw->_launchManager->setChecked(true);
    _wcw->_autocloseManager->setChecked(false);
    _wcw->_screensaverLock->setChecked(false);
    _wcw->_autoclose->setChecked(true);
    _wcw->_closeIdle->setChecked(false);
    _wcw->_idleTime->setValue(10);
    _wcw->_defaultWallet->setCurrentIndex(0);
    _wcw->_localWalletSelected->setChecked(false);
    _wcw->_localWallet->setCurrentIndex(0);
    _wcw->_accessList->clear();
    _wcw->_secretServiceAPI->setChecked(true);
    setNeedsSave(true);
}

void KWalletConfig::customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *item = _wcw->_accessList->itemAt(pos);
    if (item && item->parent()) {
        auto m = new QMenu(widget());
        m->setTitle(item->parent()->text(0));
        m->addAction(i18n("&Delete"), Qt::Key_Delete, this, &KWalletConfig::deleteEntry);
        m->exec(_wcw->_accessList->mapToGlobal(pos));
        delete m;
    }
}

void KWalletConfig::deleteEntry()
{
    QList<QTreeWidgetItem *> items = _wcw->_accessList->selectedItems();
    if (items.count() == 1 && items[0]) {
        delete items[0];
        setNeedsSave(true);
    }
}

#include "konfigurator.moc"

#include "moc_konfigurator.cpp"
