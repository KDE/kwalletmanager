/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "applicationsmanager.h"
#include "connectedappmodel.h"
#include "authorizedappmodel.h"
#include <KWallet>


ApplicationsManager::ApplicationsManager(QWidget *parent):
    QWidget(parent),
    _wallet(nullptr),
    _connectedAppsModel(nullptr),
    _authorizedAppModel(nullptr)
{
    setupUi(this);
}

ApplicationsManager::~ApplicationsManager()
{
    delete _connectedAppsModel;
    delete _authorizedAppModel;
}

void ApplicationsManager::setWallet(KWallet::Wallet *wallet)
{
    Q_ASSERT(wallet != nullptr);
    _wallet = wallet;

    delete _connectedAppsModel;
    delete _authorizedAppModel;

    // create the disconnect widget menu
    _connectedAppsModel = new ConnectedAppModel(_wallet);
    _connectedApps->setWallet(_wallet);
    _connectedApps->setModel(_connectedAppsModel);

    _authorizedAppModel = new AuthorizedAppModel(_wallet);
    _authorizedApps->setWallet(_wallet);
    _authorizedApps->setModel(_authorizedAppModel);
}


