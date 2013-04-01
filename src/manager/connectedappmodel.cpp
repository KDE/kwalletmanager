/*
   Copyright (C) 2013 Valentin Rusu <kde@rusu.info>

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

#include "connectedappmodel.h"

#include <kwallet.h>
#include <kdebug.h>


ConnectedAppModel::ConnectedAppModel(KWallet::Wallet* wallet):
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
    int row =0;
    Q_FOREACH(QString appName, _connectedApps ) {
        // for un unknown reason, kwalletd returs empty strings so lets avoid inserting them
        // FIXME: find out why kwalletd returns empty strings here
        if (appName.length()>0) {
            QStandardItem *item = new QStandardItem(appName);
            item->setEditable(false);
            setItem(row, 0, item);
            // this item will be hidden by the disconnect button, see below setIndexWidget call
            setItem(row, 1, new QStandardItem("dummy"));
            _connectedAppsIndexMap.insert(appName, QPersistentModelIndex(index(row, 0)));
            row++;
        }
    }
}

void ConnectedAppModel::removeApp(QString appName)
{
    if (_connectedAppsIndexMap.contains(appName)) {
        QPersistentModelIndex idx = _connectedAppsIndexMap[appName];
        if (idx.isValid()) {
            if (!removeRow(idx.row())) {
                kDebug() << "Remove row failed for app " << appName;
            }
        }
    } else {
        kDebug() << "Attempting to remove unknown application " << appName;
    }
}


#include "connectedappmodel.moc"
