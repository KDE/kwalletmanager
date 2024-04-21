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
    explicit RevokeAuthButton(const QString &appName, KWallet::Wallet *wallet);

Q_SIGNALS:
    void appRevoked(const QString &);

private:
    void onClicked();
    const QString _appName;
    KWallet::Wallet *const _wallet;
};

#endif // REVOKEAUTHBUTTON_H
