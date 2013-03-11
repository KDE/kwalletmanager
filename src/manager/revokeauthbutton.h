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

#ifndef REVOKEAUTHBUTTON_H
#define REVOKEAUTHBUTTON_H

#include <QPushButton>

namespace KWallet {
class Wallet;
}

class RevokeAuthButton : public QPushButton
{
    Q_OBJECT
public:
    RevokeAuthButton(const QString& appName, KWallet::Wallet *wallet);

private Q_SLOTS:
    void onClicked();

Q_SIGNALS:
    void appRevoked(QString);

private:
    QString             _appName;
    KWallet::Wallet     *_wallet;
};

#endif // REVOKEAUTHBUTTON_H
