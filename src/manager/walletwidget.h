/*
   Copyright (C) 2003-2005 George Staikos <staikos@kde.org>
   Copyright (C) 2005 Isaac Clerencia <isaac@warp.es>
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
#ifndef WALLETWIDGET_H
#define WALLETWIDGET_H

#include "ui_walletwidget.h"

namespace KWallet {
class Wallet;
}

class KWMapEditor;
class QCheckBox;
class KWalletEntryList;

class WalletWidget : public QWidget, public Ui::WalletWidget
{
public:
    explicit WalletWidget( QWidget *parent =0);

    void startWalletEditing(KWallet::Wallet *wallet);
    void forgetWallet();

private:
    KWalletEntryList        *_entryList;
    QCheckBox               *_mapEditorShowHide;
    KWMapEditor             *_mapEditor;
    QMap<QString,QString>   _currentMap; // save memory by storing
    KWallet::Wallet         *_wallet;
};

#endif // WALLETWIDGET_H
