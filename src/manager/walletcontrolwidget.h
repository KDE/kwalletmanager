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
#ifndef WALLETCONTROLWIDGET_H
#define WALLETCONTROLWIDGET_H

#include "ui_walletcontrolwidget.h"

class KWalletEditor;
class ApplicationsManager;

namespace KWallet {
class Wallet;
}

class WalletControlWidget : public QWidget, public Ui::WalletControlWidget 
{
    Q_OBJECT
public:
    WalletControlWidget(QWidget* parent, const QString& walletName);

    bool openWallet();

public Q_SLOTS:
    void onSetupWidget();
    void onOpenClose();
    void onWalletClosed();
    void updateWalletDisplay();
    void onDisconnectApplication();
    void onChangePassword();

protected:
    virtual void hideEvent(QHideEvent*);
    virtual void showEvent(QShowEvent*);

private:
    QString             _walletName;
    KWallet::Wallet*    _wallet;
    KWalletEditor*      _walletEditor;
    ApplicationsManager* _applicationsManager;
};

#endif // WALLETCONTROLWIDGET_H
