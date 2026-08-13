/*
    SPDX-FileCopyrightText: 2003 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "konfigurator.h"
#include "../kwalletmanager_version.h"

#include <KAuth/ActionReply>
#include <KAuth/ExecuteJob>
#include <KConfigGroup>
#include <KMessageBox>
#include <KPluginFactory>
#include <KWallet>
#include <QInputDialog>

#include <KAboutData>
#include <KCoreAddons>

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
    connect(_wcw->_localWalletSelected, &QCheckBox::clicked, this, &KWalletConfig::configChanged);
    connect(_wcw->_idleTime, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &KWalletConfig::configChanged);
    connect(_wcw->_launch, &QPushButton::clicked, this, &KWalletConfig::launchManager);
    connect(_wcw->_newWallet, &QPushButton::clicked, this, &KWalletConfig::newNetworkWallet);
    connect(_wcw->_newLocalWallet, &QPushButton::clicked, this, &KWalletConfig::newLocalWallet);
    connect(_wcw->_localWallet, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &KWalletConfig::configChanged);
    connect(_wcw->_defaultWallet, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &KWalletConfig::configChanged);
    connect(_wcw->_secretServiceAPI, &QCheckBox::clicked, this, &KWalletConfig::configChanged);

    QStyle *style = widget()->style();
    _wcw->launchButtonBar->setContentsMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                              0,
                                              style->pixelMetric(QStyle::PM_LayoutRightMargin),
                                              style->pixelMetric(QStyle::PM_LayoutBottomMargin));

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
    _wcw->_launchManager->setChecked(config.readEntry("Launch Manager", false));
    _wcw->_autocloseManager->setChecked(!config.readEntry("Leave Manager Open", false));
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
    _wcw->_launchManager->setChecked(true);
    _wcw->_autocloseManager->setChecked(false);
    _wcw->_autoclose->setChecked(true);
    _wcw->_closeIdle->setChecked(false);
    _wcw->_idleTime->setValue(10);
    _wcw->_defaultWallet->setCurrentIndex(0);
    _wcw->_localWalletSelected->setChecked(false);
    _wcw->_localWallet->setCurrentIndex(0);
    _wcw->_secretServiceAPI->setChecked(true);
    setNeedsSave(true);
}

#include "konfigurator.moc"

#include "moc_konfigurator.cpp"
