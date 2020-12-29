/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "authorizedappmodel.h"
#include "kwalletmanager_debug.h"
#include <KConfigGroup>
#include <KWallet>
#include <QTimer>

AuthorizedAppModel::AuthorizedAppModel(KWallet::Wallet *wallet):
    QStandardItemModel(),
    _cfg(KSharedConfig::openConfig(QStringLiteral("kwalletrc"), KConfig::NoGlobals)),
    _wallet(wallet)
{
    // TODO: handle "Auto Deny" applications
    // KConfigGroup ad(_cfg, "Auto Deny");

    KConfigGroup aa(_cfg, "Auto Allow");
    QString walletName = _wallet->walletName();
    const QStringList keys = aa.entryMap().keys();
    for (const QString &cfgWalletName : keys) {
        if (cfgWalletName == walletName) {
            const QStringList apps = aa.readEntry(cfgWalletName, QStringList());
            int row = 0;
            for (const QString &appName : apps) {
                setItem(row, 0, new QStandardItem(appName));
                setItem(row, 1, new QStandardItem(QStringLiteral("dummy"))); // this item will be hidden by the disconnect button, see below setIndexWidget call
                _authorizedAppsIndexMap.insert(appName, QPersistentModelIndex(index(row, 0)));
                row++;
            }
        }
    }
}

void AuthorizedAppModel::removeApp(const QString &appName)
{
    if (_authorizedAppsIndexMap.contains(appName)) {
        QPersistentModelIndex idx = _authorizedAppsIndexMap[appName];
        if (idx.isValid()) {
            if (!removeRow(idx.row())) {
                qCDebug(KWALLETMANAGER_LOG) << "Remove row failed for app " << appName;
            }
        }
    } else {
        qCDebug(KWALLETMANAGER_LOG) << "Attempting to remove unknown application " << appName;
    }
    QTimer::singleShot(0, this, &AuthorizedAppModel::saveConfig);
}

void AuthorizedAppModel::saveConfig()
{
    QStringList appList;
    appList.reserve(rowCount());
    for (int r = 0; r < rowCount(); r++) {
        appList << item(r)->text();
    }
    QString walletName = _wallet->walletName();
    KConfigGroup config(_cfg, "Auto Allow");
    config.deleteEntry(walletName);
    config.writeEntry(_wallet->walletName(), appList);
    _cfg->sync();
}


