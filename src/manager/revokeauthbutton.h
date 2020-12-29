/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REVOKEAUTHBUTTON_H
#define REVOKEAUTHBUTTON_H

#include <QPushButton>

namespace KWallet
{
class Wallet;
}

class RevokeAuthButton : public QPushButton
{
    Q_OBJECT
public:
    RevokeAuthButton(const QString &appName, KWallet::Wallet *wallet);

private Q_SLOTS:
    void onClicked();

Q_SIGNALS:
    void appRevoked(const QString&);

private:
    QString             _appName;
    KWallet::Wallet     *_wallet = nullptr;
};

#endif // REVOKEAUTHBUTTON_H
