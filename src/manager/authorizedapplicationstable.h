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


#ifndef AUTHORIZEDAPPLICATIONSTABLE_H
#define AUTHORIZEDAPPLICATIONSTABLE_H

#include <QTableView>

namespace KWallet {
class Wallet;
}

class AuthorizedApplicationsTable : public QTableView
{
    Q_OBJECT
public:
    explicit AuthorizedApplicationsTable(QWidget* parent);

    virtual void setModel(QAbstractItemModel *model);
    void setWallet(KWallet::Wallet *wallet);

protected:
    virtual void resizeEvent(QResizeEvent *resizeEvent);

private:
    KWallet::Wallet     *_wallet;
};

#endif // AUTHORIZEDAPPLICATIONSTABLE_H
