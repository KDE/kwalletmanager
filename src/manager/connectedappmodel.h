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

#ifndef CONNECTEDAPPMODEL_H
#define CONNECTEDAPPMODEL_H

#include <QStandardItemModel>

namespace KWallet {
class Wallet;
}

class ConnectedAppModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit ConnectedAppModel(KWallet::Wallet *wallet);

public Q_SLOTS:
    void refresh();
    void removeApp(QString);

private:
    KWallet::Wallet                         *_wallet;
    QStringList                             _connectedApps;
    QMap<QString, QPersistentModelIndex>    _connectedAppsIndexMap;
};


#endif // CONNECTEDAPPMODEL_H
