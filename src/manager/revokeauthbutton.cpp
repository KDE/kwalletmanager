/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "revokeauthbutton.h"
#include <KLocalizedString>

RevokeAuthButton::RevokeAuthButton(const QString &appName, KWallet::Wallet *wallet):
    QPushButton(),
    _appName(appName),
    _wallet(wallet)
{
    setObjectName(QStringLiteral("Revoke_%1").arg(appName));
    setText(i18n("Revoke Authorization"));
    connect(this, &RevokeAuthButton::clicked, this, &RevokeAuthButton::onClicked);
}

void RevokeAuthButton::onClicked()
{
    Q_EMIT appRevoked(_appName);
}


