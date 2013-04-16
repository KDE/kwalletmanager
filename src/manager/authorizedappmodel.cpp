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

#include "authorizedappmodel.h"

#include <kconfiggroup.h>
#include <kwallet.h>
#include <kdebug.h>
#include <QTimer>

AuthorizedAppModel::AuthorizedAppModel(KWallet::Wallet* wallet): 
    QStandardItemModel(),
    _cfg(KSharedConfig::openConfig(QLatin1String( "kwalletrc" ), KConfig::NoGlobals)),
    _wallet(wallet)
{
    // TODO: handle "Auto Deny" applications
    // KConfigGroup ad(_cfg, "Auto Deny");

    KConfigGroup aa(_cfg, "Auto Allow");
    QString walletName = _wallet->walletName();
    const QStringList keys = aa.entryMap().keys();
    Q_FOREACH(QString cfgWalletName, keys) {
        if (cfgWalletName == walletName) {
            const QStringList apps = aa.readEntry(cfgWalletName, QStringList());
            int row = 0;
            Q_FOREACH(QString appName, apps) {
                setItem(row, 0, new QStandardItem(appName));
                setItem(row, 1, new QStandardItem("dummy")); // this item will be hidden by the disconnect button, see below setIndexWidget call
                _authorizedAppsIndexMap.insert(appName, QPersistentModelIndex(index(row, 0)));
                row++;
            }
        }
    }
}

void AuthorizedAppModel::removeApp(QString appName)
{
    if (_authorizedAppsIndexMap.contains(appName)) {
        QPersistentModelIndex idx = _authorizedAppsIndexMap[appName];
        if (idx.isValid()) {
            if (!removeRow(idx.row())) {
                kDebug() << "Remove row failed for app " << appName;
            }
        }
    } else {
        kDebug() << "Attempting to remove unknown application " << appName;
    }
    QTimer::singleShot(0, this, SLOT(saveConfig()));
}

void AuthorizedAppModel::saveConfig()
{
    QStringList appList;
    for (int r=0; r <rowCount(); r++) {
        appList << item(r)->text();
    }
    QString walletName = _wallet->walletName();
    KConfigGroup config(_cfg, "Auto Allow");
    config.deleteEntry(walletName);
    config.writeEntry(_wallet->walletName(), appList);
    _cfg->sync();
}

#include "authorizedappmodel.moc"
