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

#include "disconnectappbutton.h"

#include <klocalizedstring.h>
#include <kwallet.h>

DisconnectAppButton::DisconnectAppButton(const QString& appName, KWallet::Wallet* wallet) :
        _appName(appName), _wallet(wallet) 
{
    setObjectName(QString("Disconnect_%1").arg(appName));
    setText(i18n("Disconnect"));
    connect(this, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
}

void DisconnectAppButton::onClicked()
{
    if (_wallet->disconnectApplication(_wallet->walletName(), _appName)) {
        emit appDisconnected(_appName);
    }
}

#include "disconnectappbutton.moc"
