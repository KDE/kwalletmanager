/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2013 Valentin Rusu <kde@rusu.info>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kwalletmanagerwidgetitem.h"
#include "walletcontrolwidget.h"

#include <kwallet.h>
#include <kicon.h>

KWalletManagerWidgetItem::KWalletManagerWidgetItem(QWidget* widgetParent, const QString& walletName):
    KPageWidgetItem( _controlWidget = new WalletControlWidget(widgetParent, walletName), walletName ),
    _walletName(walletName)
{
    updateWalletDisplay();
}

void KWalletManagerWidgetItem::updateWalletDisplay()
{
    if (KWallet::Wallet::isOpen(_walletName)) {
        setIcon( KIcon( QLatin1String("wallet-open") ) );
    } else {
        setIcon( KIcon( QLatin1String("wallet-closed") ) );
    }
    _controlWidget->updateWalletDisplay();
}

bool KWalletManagerWidgetItem::openWallet()
{
    return _controlWidget->openWallet();
}

#include "kwalletmanagerwidgetitem.moc"
