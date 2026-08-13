/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "applicationsmanager.h"
#include "connectedappmodel.h"
#include <KCoreAddons>
#include <KWallet>

ApplicationsManager::ApplicationsManager(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}

ApplicationsManager::~ApplicationsManager()
{
    delete _connectedAppsModel;
}

void ApplicationsManager::setWallet(KWallet::Wallet *wallet)
{
    Q_ASSERT(wallet != nullptr);
    _wallet = wallet;

    delete _connectedAppsModel;

    // create the disconnect widget menu
    _connectedAppsModel = new ConnectedAppModel(_wallet);
    _connectedApps->setWallet(_wallet);
    _connectedApps->setModel(_connectedAppsModel);
}

#include "moc_applicationsmanager.cpp"
