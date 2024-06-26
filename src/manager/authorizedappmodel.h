/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AUTHORIZEDAPPMODEL_H
#define AUTHORIZEDAPPMODEL_H

#include <KSharedConfig>
#include <QStandardItemModel>

namespace KWallet
{
class Wallet;
}

class AuthorizedAppModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit AuthorizedAppModel(KWallet::Wallet *wallet);

public Q_SLOTS:
    void removeApp(const QString &);

private Q_SLOTS:
    void saveConfig();

private:
    KSharedConfig::Ptr _cfg;
    KWallet::Wallet *const _wallet;
    QMap<QString, QPersistentModelIndex> _authorizedAppsIndexMap;
};

#endif // AUTHORIZEDAPPMODEL_H
