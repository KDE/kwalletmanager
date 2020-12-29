/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DISCONNECTAPPBUTTON_H
#define DISCONNECTAPPBUTTON_H

#include <QPushButton>

namespace KWallet
{
class Wallet;
}

class DisconnectAppButton : public QPushButton
{
    Q_OBJECT
public:
    explicit DisconnectAppButton(const QString &appName, KWallet::Wallet *wallet);

Q_SIGNALS:
    void appDisconnected(const QString &);

private Q_SLOTS:
    void onClicked();

private:
    QString             _appName;
    KWallet::Wallet     *_wallet = nullptr;
};

#endif // DISCONNECTAPPBUTTON_H
