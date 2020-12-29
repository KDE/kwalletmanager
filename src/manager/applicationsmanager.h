/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef APPLICATIONSMANAGER_H
#define APPLICATIONSMANAGER_H

#include "ui_applicationsmanager.h"
#include <QWidget>

class AuthorizedAppModel;
class ConnectedAppModel;
namespace KWallet
{
class Wallet;
}

class ApplicationsManager : public QWidget, public Ui::ApplicationsManager
{
    Q_OBJECT
public:
    explicit ApplicationsManager(QWidget *parent);
    ~ApplicationsManager() override;

    void setWallet(KWallet::Wallet *wallet);

private:
    KWallet::Wallet     *_wallet;
    ConnectedAppModel   *_connectedAppsModel;
    AuthorizedAppModel  *_authorizedAppModel;
};

#endif // APPLICATIONSMANAGER_H
