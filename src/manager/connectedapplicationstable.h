/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CONNECTEDAPPLICATIONSTABLE_H
#define CONNECTEDAPPLICATIONSTABLE_H

#include <KWallet>
#include <QTableView>

class ConnectedApplicationsTable : public QTableView
{
    Q_OBJECT
public:
    explicit ConnectedApplicationsTable(QWidget *parent);

    void setModel(QAbstractItemModel *model) override;
    void setWallet(KWallet::Wallet *wallet);

protected:
    void resizeEvent(QResizeEvent *resizeEvent) override;

private:
    KWallet::Wallet     *_wallet = nullptr;
};

#endif // CONNECTEDAPPLICATIONSTABLE_H
