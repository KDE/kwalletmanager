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


#include "connectedapplicationstable.h"
#include "disconnectappbutton.h"
#include "connectedappmodel.h"

#include <klocalizedstring.h>
#include <QPushButton>

ConnectedApplicationsTable::ConnectedApplicationsTable(QWidget* parent):
    QTableView(parent),
    _wallet(0)
{
}

void ConnectedApplicationsTable::setWallet(KWallet::Wallet* wallet)
{
    _wallet = wallet;
}

void ConnectedApplicationsTable::setModel(QAbstractItemModel* model)
{
    Q_ASSERT(_wallet != 0);

    ConnectedAppModel *appModel = qobject_cast<ConnectedAppModel*>(model);
    Q_ASSERT(appModel != 0);

    QTableView::setModel(model);
    for (int row =0; row < model->rowCount(); row++) {
        DisconnectAppButton *btn = new DisconnectAppButton( model->index(row, 0).data().toString() , _wallet);
        btn->setFixedHeight(btn->sizeHint().height());
        setRowHeight(row, btn->height());
        setIndexWidget( model->index(row, 1), btn);
        connect(btn, SIGNAL(appDisconnected(QString)), appModel, SLOT(removeApp(QString)));
    }
}

void ConnectedApplicationsTable::resizeEvent(QResizeEvent* resizeEvent)
{
    // this will keep disconnect buttons column at it's minimum size and
    // make the application names take the reminder of the horizontal space
    resizeColumnsToContents();
    int appColumnSize = contentsRect().width() - columnWidth(1) - 50;
    setColumnWidth(0, appColumnSize);
    QAbstractItemView::resizeEvent(resizeEvent);
}

#include "connectedapplicationstable.moc"
