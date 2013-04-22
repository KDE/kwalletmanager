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

#include "authorizedapplicationstable.h"
#include "authorizedappmodel.h"
#include "revokeauthbutton.h"

AuthorizedApplicationsTable::AuthorizedApplicationsTable(QWidget* parent) :
    QTableView(parent),
    _wallet(0)
{
}

void AuthorizedApplicationsTable::setWallet(KWallet::Wallet* wallet)
{
    _wallet = wallet;
}

void AuthorizedApplicationsTable::setModel(QAbstractItemModel* model)
{
    Q_ASSERT(_wallet != 0);

    AuthorizedAppModel *appModel = qobject_cast<AuthorizedAppModel*>(model);
    Q_ASSERT(appModel != 0);

    QTableView::setModel(model);
    for (int row =0; row < model->rowCount(); row++) {
        RevokeAuthButton *btn = new RevokeAuthButton( model->index(row, 0).data().toString() , _wallet);
        btn->setFixedHeight(btn->sizeHint().height());
        setRowHeight(row, btn->height());
        setIndexWidget( model->index(row, 1), btn);
        connect(btn, SIGNAL(appRevoked(QString)), appModel, SLOT(removeApp(QString)));
    }
}

void AuthorizedApplicationsTable::resizeEvent(QResizeEvent* resizeEvent)
{
    // this will keep disconnect buttons column at it's minimum size and
    // make the application names take the reminder of the horizontal space
    resizeColumnsToContents();
    int appColumnSize = contentsRect().width() - columnWidth(1) - 50;
    setColumnWidth(0, appColumnSize);
    QAbstractItemView::resizeEvent(resizeEvent);
}

#include "authorizedapplicationstable.moc"
