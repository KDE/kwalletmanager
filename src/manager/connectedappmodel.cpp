/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "connectedappmodel.h"
#include "kwalletmanager_debug.h"

#include <KWallet>


ConnectedAppModel::ConnectedAppModel(KWallet::Wallet *wallet):
    QStandardItemModel(),
    _wallet(wallet)
{
    refresh();
}

void ConnectedAppModel::refresh()
{
    clear();
    _connectedAppsIndexMap.clear();

    _connectedApps = KWallet::Wallet::users(_wallet->walletName());
    int row = 0;
    for (const QString &appName : qAsConst(_connectedApps)) {
        // for un unknown reason, kwalletd returs empty strings so lets avoid inserting them
        // FIXME: find out why kwalletd returns empty strings here
        if (!appName.isEmpty()) {
            auto item = new QStandardItem(appName);
            item->setEditable(false);
            setItem(row, 0, item);
            // this item will be hidden by the disconnect button, see below setIndexWidget call
            setItem(row, 1, new QStandardItem(QStringLiteral("dummy")));
            _connectedAppsIndexMap.insert(appName, QPersistentModelIndex(index(row, 0)));
            row++;
        }
    }
}

void ConnectedAppModel::removeApp(const QString &appName)
{
    if (_connectedAppsIndexMap.contains(appName)) {
        QPersistentModelIndex idx = _connectedAppsIndexMap[appName];
        if (idx.isValid()) {
            if (!removeRow(idx.row())) {
                qCDebug(KWALLETMANAGER_LOG) << "Remove row failed for app " << appName;
            }
        }
    } else {
        qCDebug(KWALLETMANAGER_LOG) << "Attempting to remove unknown application " << appName;
    }
}


