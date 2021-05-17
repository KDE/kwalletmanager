/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "disconnectappbutton.h"

#include <KLocalizedString>
#include <KWallet>

DisconnectAppButton::DisconnectAppButton(const QString &appName, KWallet::Wallet *wallet) :
    _appName(appName), _wallet(wallet)
{
    setObjectName(QStringLiteral("Disconnect_%1").arg(appName));
    setText(i18n("Disconnect"));
    connect(this, &DisconnectAppButton::clicked, this, &DisconnectAppButton::onClicked);
}

void DisconnectAppButton::onClicked()
{
    if (_wallet->disconnectApplication(_wallet->walletName(), _appName)) {
        Q_EMIT appDisconnected(_appName);
    }
}


