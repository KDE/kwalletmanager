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

#ifndef APPLICATIONSMANAGER_H
#define APPLICATIONSMANAGER_H

#include "ui_applicationsmanager.h"
#include <QWidget>

class AuthorizedAppModel;
class ConnectedAppModel;
namespace KWallet {
class Wallet;
}


class ApplicationsManager : public QWidget, public Ui::ApplicationsManager
{
    Q_OBJECT
public:
    ApplicationsManager(QWidget *parent);
    virtual ~ApplicationsManager();

    void setWallet(KWallet::Wallet *wallet);

private:
    KWallet::Wallet     *_wallet;
    ConnectedAppModel   *_connectedAppsModel;
    AuthorizedAppModel  *_authorizedAppModel;
};

#endif // APPLICATIONSMANAGER_H
