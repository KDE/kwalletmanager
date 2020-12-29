/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AUTHORIZEDAPPLICATIONSTABLE_H
#define AUTHORIZEDAPPLICATIONSTABLE_H

#include <QTableView>

namespace KWallet
{
class Wallet;
}

class AuthorizedApplicationsTable : public QTableView
{
    Q_OBJECT
public:
    explicit AuthorizedApplicationsTable(QWidget *parent);

    void setModel(QAbstractItemModel *model) override;
    void setWallet(KWallet::Wallet *wallet);

protected:
    void resizeEvent(QResizeEvent *resizeEvent) override;

private:
    KWallet::Wallet     *_wallet;
};

#endif // AUTHORIZEDAPPLICATIONSTABLE_H
