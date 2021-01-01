/*
    SPDX-FileCopyrightText: 2003 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWALLETPOPUP_H
#define KWALLETPOPUP_H

#include <QMenu>


class KWalletPopup : public QMenu
{
    Q_OBJECT

public:
    explicit KWalletPopup(const QString &wallet, QWidget *parent = nullptr, const QString &name = QString());
    ~KWalletPopup() override;

public Q_SLOTS:
    void openWallet();
    void deleteWallet();
    void closeWallet();
    void createWallet();
    void changeWalletPassword();
    void disconnectApp();

Q_SIGNALS:
    void walletOpened(const QString &walletName);
    void walletClosed(const QString &walletName);
    void walletDeleted(const QString &walletName);
    void walletCreated();
    void walletChangePassword(const QString &walletName);

private:
    QString _walletName;
};

#endif
