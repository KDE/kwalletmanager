/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONNECTEDAPPMODEL_H
#define CONNECTEDAPPMODEL_H

#include <QStandardItemModel>

namespace KWallet
{
class Wallet;
}

class ConnectedAppModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit ConnectedAppModel(KWallet::Wallet *wallet);

public Q_SLOTS:
    void refresh();
    void removeApp(const QString &);

private:
    KWallet::Wallet                         *_wallet = nullptr;
    QStringList                             _connectedApps;
    QMap<QString, QPersistentModelIndex>    _connectedAppsIndexMap;
};

#endif // CONNECTEDAPPMODEL_H
