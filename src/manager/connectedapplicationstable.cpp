/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "connectedapplicationstable.h"
#include "disconnectappbutton.h"
#include "connectedappmodel.h"

#include <KLocalizedString>
#include <QPushButton>

ConnectedApplicationsTable::ConnectedApplicationsTable(QWidget *parent):
    QTableView(parent)
{
}

void ConnectedApplicationsTable::setWallet(KWallet::Wallet *wallet)
{
    _wallet = wallet;
}

void ConnectedApplicationsTable::setModel(QAbstractItemModel *model)
{
    Q_ASSERT(_wallet != nullptr);

    auto appModel = qobject_cast<ConnectedAppModel *>(model);
    Q_ASSERT(appModel != nullptr);

    QTableView::setModel(model);
    const int numberRow(model->rowCount());
    for (int row = 0; row < numberRow; row++) {
        auto btn = new DisconnectAppButton(model->index(row, 0).data().toString(), _wallet);
        btn->setFixedHeight(btn->sizeHint().height());
        setRowHeight(row, btn->height());
        setIndexWidget(model->index(row, 1), btn);
        connect(btn, &DisconnectAppButton::appDisconnected, appModel, &ConnectedAppModel::removeApp);
    }
}

void ConnectedApplicationsTable::resizeEvent(QResizeEvent *resizeEvent)
{
    // this will keep disconnect buttons column at it's minimum size and
    // make the application names take the reminder of the horizontal space
    resizeColumnsToContents();
    const int appColumnSize = contentsRect().width() - columnWidth(1) - 50;
    setColumnWidth(0, appColumnSize);
    QAbstractItemView::resizeEvent(resizeEvent);
}


