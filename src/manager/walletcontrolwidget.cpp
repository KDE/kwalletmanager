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

#include "walletcontrolwidget.h"
#include "walletwidget.h"

#include <QPropertyAnimation>
#include <QTimer>
#include <kwallet.h>
#include <kmessagebox.h>

WalletControlWidget::WalletControlWidget(QWidget* parent, const QString& walletName):
    QWidget(parent),
    _walletName(walletName),
    _wallet(0),
    _walletWidget(0)
{
    setupUi(this);

    _walletWidget = new WalletWidget(_editorFrame);
    _walletWidget->setVisible(false);
    _editorFrameLayout->addWidget(_walletWidget);

    QTimer::singleShot(1, this, SLOT(onSetupWidget()));
}

void WalletControlWidget::onSetupWidget()
{
    if (KWallet::Wallet::isOpen(_walletName)) {
        _wallet = KWallet::Wallet::openWallet(_walletName, winId());
        connect(_wallet, SIGNAL(walletClosed()), this, SLOT(onWalletClosed()));
        _openClose->setText(tr2i18n("Close", 0));
        _openClose->setArrowType(Qt::UpArrow);
        _walletWidget->setVisible(true);
        _changePassword->setEnabled(true);
        _disconnect->setEnabled(true);
        _delete->setEnabled(true);
    } else {
        _walletWidget->setVisible(false);
        _changePassword->setEnabled(false);
        _disconnect->setEnabled(false);
        _delete->setEnabled(false);
    }
}

void WalletControlWidget::onOpenClose()
{
    // TODO create some fancy animation here to make _walletWidget appear or dissapear in a fancy way
    if (_wallet) {
        // Wallet is open, attempt close it
        int rc = KWallet::Wallet::closeWallet(_walletName, false);
        if (rc != 0) {
            rc = KMessageBox::warningYesNo(this, i18n("Unable to close wallet cleanly. It is probably in use by other applications. Do you wish to force it closed?"), QString(), KGuiItem(i18n("Force Closure")), KGuiItem(i18n("Do Not Force")));
            if (rc == KMessageBox::Yes) {
                rc = KWallet::Wallet::closeWallet(_walletName, true);
                if (rc != 0) {
                    KMessageBox::sorry(this, i18n("Unable to force the wallet closed. Error code was %1.", rc));
                }
            }
        }
    }
    else {
        _wallet = KWallet::Wallet::openWallet(_walletName, false);
        onSetupWidget();
    }
}

void WalletControlWidget::onWalletClosed()
{
    _wallet = 0;
    onSetupWidget();
}

void WalletControlWidget::onChangePassword()
{
    KWallet::Wallet::changePassword(_walletName, winId());
}
