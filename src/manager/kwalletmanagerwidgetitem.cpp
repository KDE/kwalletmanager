/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwalletmanagerwidgetitem.h"
#include "walletcontrolwidget.h"

#include <KWallet>
#include <QIcon>

KWalletManagerWidgetItem::KWalletManagerWidgetItem(QWidget *widgetParent, const QString &walletName):
    KPageWidgetItem(_controlWidget = new WalletControlWidget(widgetParent, walletName), walletName),
    _walletName(walletName)
{
    updateWalletDisplay();
}

void KWalletManagerWidgetItem::updateWalletDisplay()
{
    if (KWallet::Wallet::isOpen(_walletName)) {
        setIcon(QIcon::fromTheme(QStringLiteral("wallet-open")));
    } else {
        setIcon(QIcon::fromTheme(QStringLiteral("wallet-closed")));
    }
    _controlWidget->updateWalletDisplay();
}

bool KWalletManagerWidgetItem::openWallet()
{
    return _controlWidget->openWallet();
}

bool KWalletManagerWidgetItem::hasUnsavedChanges() const
{
    return (_controlWidget ? _controlWidget->hasUnsavedChanges() : false);
}
