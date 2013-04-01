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

#include "applicationsmanager.h"
#include "connectedappmodel.h"
#include "authorizedappmodel.h"
#include "kwallet.h"

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStandardItemModel>
#include <kdebug.h>

ApplicationsManager::ApplicationsManager(QWidget* parent):
    QWidget(parent),
    _wallet(0),
    _connectedAppsModel(0),
    _authorizedAppModel(0)
{
    setupUi(this);
}

ApplicationsManager::~ApplicationsManager()
{
    delete _connectedAppsModel;
    delete _authorizedAppModel;
}

void ApplicationsManager::setWallet(KWallet::Wallet* wallet)
{
    Q_ASSERT(wallet != 0);
    _wallet = wallet;

    // create the disconnect widget menu
    _connectedAppsModel = new ConnectedAppModel(_wallet);
    _connectedApps->setWallet(_wallet);
    _connectedApps->setModel(_connectedAppsModel);

    _authorizedAppModel = new AuthorizedAppModel(_wallet);
    _authorizedApps->setWallet(_wallet);
    _authorizedApps->setModel(_authorizedAppModel);
}

#include "applicationsmanager.moc"
