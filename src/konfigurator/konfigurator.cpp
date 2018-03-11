/*
   Copyright (C) 2003 George Staikos <staikos@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#include "../kwalletmanager_version.h"
#include "konfigurator.h"

#include <ksharedconfig.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <qinputdialog.h>
#include <kwallet.h>
#include <kauthaction.h>
#include <kauthactionreply.h>
#include <kauthexecutejob.h>
#include <ktoolinvocation.h>
#include <kconfiggroup.h>
#include <kmessagebox.h>

#include <kaboutdata.h>

#include <QDebug>
#include <QCheckBox>
#include <QMenu>
#include <QPushButton>
#include <QtDBus>
#include <QVBoxLayout>

#define KWALLETMANAGERINTERFACE "org.kde.KWallet"

K_PLUGIN_FACTORY(KWalletFactory, registerPlugin<KWalletConfig>();)

KWalletConfig::KWalletConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args),
      _cfg(KSharedConfig::openConfig(QStringLiteral("kwalletrc"), KConfig::NoGlobals))
{
    KAboutData *about = new KAboutData(QStringLiteral("kcmkwallet5"),
                                       i18n("KDE Wallet Control Module"),
                                       QStringLiteral(KWALLETMANAGER_VERSION_STRING),
                                       QString(),
                                       KAboutLicense::GPL,
                                       i18n("(c) 2003 George Staikos"));
    about->addAuthor(i18n("George Staikos"),
                     QString(),
                     QStringLiteral("staikos@kde.org"));
    setAboutData(about);
    setNeedsAuthorization(true);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setMargin(0);
    _wcw = new WalletConfigWidget(this);
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

    _wcw->_accessList->setAllColumnsShowFocus(true);
    _wcw->_accessList->setContextMenuPolicy(Qt::CustomContextMenu);
    updateWalletLists();

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.kwalletmanager"))) {
        _wcw->_launch->hide();
    }

}

KWalletConfig::~KWalletConfig()
{
}

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

    const QString n = QInputDialog::getText(this, i18n("New Wallet"),
                                            i18n("Please choose a name for the new wallet:"),
                                            QLineEdit::Normal, QString(),
                                            &ok);

    if (!ok) {
        return QString();
    }

    KWallet::Wallet *w = KWallet::Wallet::openWallet(n, topLevelWidget()->winId());
    if (!w) {
        return QString();
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

    emit changed(true);
}

void KWalletConfig::newNetworkWallet()
{
    const QString n = newWallet();
    if (n.trimmed().isEmpty()) {
        return;
    }

    updateWalletLists();

    _wcw->_defaultWallet->setCurrentIndex(_wcw->_defaultWallet->findText(n));

    emit changed(true);
}

void KWalletConfig::launchManager()
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.kwalletmanager5"))) {
        QProcess::startDetached(QStringLiteral("kwalletmanager5 --show"));
    } else {
        QDBusInterface kwalletd(QStringLiteral("org.kde.kwalletmanager5"), QStringLiteral("/kwalletmanager5/MainWindow_1"));
        kwalletd.call(QStringLiteral("show"));
        kwalletd.call(QStringLiteral("raise"));
    }
}

void KWalletConfig::configChanged()
{
    emit changed(true);
}

void KWalletConfig::load()
{
    KConfigGroup config(_cfg, "Wallet");
    _wcw->_enabled->setChecked(config.readEntry("Enabled", true));
    _wcw->_openPrompt->setChecked(config.readEntry("Prompt on Open", false));
    _wcw->_launchManager->setChecked(config.readEntry("Launch Manager", false));
    _wcw->_autocloseManager->setChecked(! config.readEntry("Leave Manager Open", false));
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
    KConfigGroup ad(_cfg, "Auto Deny");
    KConfigGroup aa(_cfg, "Auto Allow");
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
            KConfigGroup cfgAllow = KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group("Auto Allow");
            cfgAllow.deleteEntry(walletName);

            KConfigGroup cfgDeny = KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group("Auto Deny");
            cfgDeny.deleteEntry(walletName);
            continue;
        }

        const QStringList apps = aa.readEntry(*i, QStringList());
        const QStringList denyapps = ad.readEntry(*i, QStringList());
        denykeys.removeAll(walletName);
        QTreeWidgetItem *twi = new QTreeWidgetItem(_wcw->_accessList, QStringList() << walletName);

        for (QStringList::const_iterator j = apps.begin(), end = apps.end(); j != end; ++j) {
            new QTreeWidgetItem(twi, QStringList() << QString() << *j << i18n("Always Allow"));
        }
        for (QStringList::const_iterator j = denyapps.begin(), end = denyapps.end(); j != end; ++j) {
            new QTreeWidgetItem(twi, QStringList() << QString() << *j << i18n("Always Deny"));
        }
    }
    for (QStringList::const_iterator i = denykeys.constBegin(), denykeysEnd = denykeys.constEnd(); i != denykeysEnd; ++i) {
        const QStringList denyapps = ad.readEntry(*i, QStringList());
        QTreeWidgetItem *twi = new QTreeWidgetItem(_wcw->_accessList, QStringList() << *i);
        for (QStringList::const_iterator j = denyapps.begin(), denyappsEnd = denyapps.end(); j != denyappsEnd; ++j) {
            new QTreeWidgetItem(twi, QStringList() << QString() << *j << i18n("Always Deny"));
        }
    }
    _wcw->_accessList->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    emit changed(false);
}

void KWalletConfig::save()
{
    QVariantMap args;
    KAuth::Action action = authAction();
    if (!action.isValid()) {
        qDebug() << "There's no authAction, not saving settings";
        return;
    }
    action.setArguments(args);

    KAuth::ExecuteJob *j = action.execute();

    if (!j->exec()) {
        if (j->error() == KAuth::ActionReply::AuthorizationDeniedError) {
            KMessageBox::error(this, i18n("Permission denied."), i18n("KDE Wallet Control Module"));
        } else {
            KMessageBox::error(this, i18n("Error while authenticating action:\n%1", j->errorString()), i18n("KDE Wallet Control Module"));
        }
        load();
        return;
    }

    KConfigGroup config(_cfg, "Wallet");
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
    _cfg->deleteGroup("Auto Allow");
    _cfg->deleteGroup("Auto Deny");
    config  = _cfg->group("Auto Allow");
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

    config = _cfg->group("Auto Deny");
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

    _cfg->sync();

    // this restarts kwalletd if necessary
    QDBusInterface kwalletd(QStringLiteral("org.kde.kwalletd5"), QStringLiteral("/modules/kwalletd"), QStringLiteral(KWALLETMANAGERINTERFACE));
    // if wallet was deactivated, then kwalletd will exit upon start so check
    // the status before invoking reconfigure
    if (kwalletd.isValid()) {
        // this will eventually make kwalletd exit upon deactivation
        kwalletd.call(QStringLiteral("reconfigure"));
    }

    emit changed(false);
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
    emit changed(true);
}

QString KWalletConfig::quickHelp() const
{
    return i18n("This configuration module allows you to configure the KDE wallet system.");
}

void KWalletConfig::customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *item = _wcw->_accessList->itemAt(pos);
    if (item && item->parent()) {
        QMenu *m = new QMenu(this);
        m->setTitle(item->parent()->text(0));
        m->addAction(i18n("&Delete"), this, SLOT(deleteEntry()), Qt::Key_Delete);
        m->exec(_wcw->_accessList->mapToGlobal(pos));
        delete m;
    }
}

void KWalletConfig::deleteEntry()
{
    QList<QTreeWidgetItem *> items = _wcw->_accessList->selectedItems();
    if (items.count() == 1 && items[0]) {
        delete items[0];
        emit changed(true);
    }
}

#include "konfigurator.moc"
