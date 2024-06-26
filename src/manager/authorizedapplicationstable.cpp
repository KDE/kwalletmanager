/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "authorizedapplicationstable.h"
#include "authorizedappmodel.h"
#include "revokeauthbutton.h"

AuthorizedApplicationsTable::AuthorizedApplicationsTable(QWidget *parent)
    : QTableView(parent)
{
}

void AuthorizedApplicationsTable::setWallet(KWallet::Wallet *wallet)
{
    _wallet = wallet;
}

void AuthorizedApplicationsTable::setModel(QAbstractItemModel *model)
{
    Q_ASSERT(_wallet != nullptr);

    auto appModel = qobject_cast<AuthorizedAppModel *>(model);
    Q_ASSERT(appModel != nullptr);

    QTableView::setModel(model);
    const int numberRow(model->rowCount());
    for (int row = 0; row < numberRow; row++) {
        auto btn = new RevokeAuthButton(model->index(row, 0).data().toString(), _wallet);
        btn->setFixedHeight(btn->sizeHint().height());
        setRowHeight(row, btn->height());
        setIndexWidget(model->index(row, 1), btn);
        connect(btn, &RevokeAuthButton::appRevoked, appModel, &AuthorizedAppModel::removeApp);
    }
}

void AuthorizedApplicationsTable::resizeEvent(QResizeEvent *resizeEvent)
{
    // this will keep disconnect buttons column at it's minimum size and
    // make the application names take the reminder of the horizontal space
    resizeColumnsToContents();
    const int appColumnSize = contentsRect().width() - columnWidth(1) - 50;
    setColumnWidth(0, appColumnSize);
    QAbstractItemView::resizeEvent(resizeEvent);
}

#include "moc_authorizedapplicationstable.cpp"
