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
#include "kwalleteditor.h"

#include <QPropertyAnimation>
#include <QTimer>
#include <QFrame>
#include <QToolButton>
#include <kwallet.h>
#include <kmessagebox.h>
#include <kmenu.h>

WalletControlWidget::WalletControlWidget(QWidget* parent, const QString& walletName):
    QWidget(parent),
    _walletName(walletName),
    _wallet(0),
    _walletEditor(0)
{
    setupUi(this);

    QTimer::singleShot(1, this, SLOT(onSetupWidget()));
}

void WalletControlWidget::onSetupWidget()
{
    if (KWallet::Wallet::isOpen(_walletName)) {
        if (0 == _wallet) {
            _wallet = KWallet::Wallet::openWallet(_walletName, winId());
            Q_ASSERT(_wallet != 0);
        }
        if (_wallet) {
            connect(_wallet, SIGNAL(walletClosed()), this, SLOT(onWalletClosed()));
            _openClose->setText(tr2i18n("&Close", 0));
            _openClose->setArrowType(Qt::UpArrow);

            _walletEditor = new KWalletEditor(_editorFrame, _wallet, false);
            _editorFrameLayout->addWidget(_walletEditor);
            _walletEditor->setVisible(true);
            _walletEditor->setControlWidget(_control);

            _control->setEnabled(true);
        }
    } else {
        _openClose->setText(tr2i18n("&Open", 0));
        _openClose->setArrowType(Qt::DownArrow);

        if (_walletEditor) {
            _walletEditor->setVisible(false);
            delete _walletEditor, _walletEditor =0;
        }
        _control->setEnabled(false);
    }
}

void WalletControlWidget::onOpenClose()
{
    // TODO create some fancy animation here to make _walletEditor appear or dissapear in a fancy way
    if (_wallet) {
        // Wallet is open, attempt close it
        int rc = KWallet::Wallet::closeWallet(_walletName, false);
        if (rc != 0) {
            rc = KMessageBox::warningYesNo(this, i18n("Unable to close wallet cleanly. It is probably in use by other applications. Do you wish to force it closed?"), QString(), KGuiItem(i18n("Force Closure")), KGuiItem(i18n("Do Not Force")));
            if (rc == KMessageBox::Yes) {
                rc = KWallet::Wallet::closeWallet(_walletName, true);
                if (rc != 0) {
                    KMessageBox::sorry(this, i18n("Unable to force the wallet closed. Error code was %1.", rc));
                } else {
                    _wallet = 0;
                }
            }
        } else {
            _wallet = 0;
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

void WalletControlWidget::updateWalletDisplay()
{
    QList<QAction*> existingActions = _disconnect->actions();
    QList<QAction*>::const_iterator i = existingActions.constBegin();
    QList<QAction*>::const_iterator ie = existingActions.constEnd();
    for ( ; i != ie; i++ ) {
        _disconnect->removeAction(*i);
    }

    // create the disconnect widget menu
    const QStringList ul = KWallet::Wallet::users(_walletName);
    if (!ul.isEmpty()) {
        for (QStringList::const_iterator it = ul.begin(); it != ul.end(); ++it) {
            QAction *a = new QAction(*it, this);
            connect(a, SIGNAL(triggered()), this, SLOT(onDisconnectApplication()));
            _disconnect->addAction(a);
            a->setData(*it);
        }
    }
}

void WalletControlWidget::onDisconnectApplication()
{
    QAction *a = qobject_cast<QAction *>(sender());
    Q_ASSERT(a);
    if (a)  {
        KWallet::Wallet::disconnectApplication(_walletName, a->data().toString());
    }
}

void WalletControlWidget::onDisconnectPressed()
{
    _disconnect->showMenu();
}

void WalletControlWidget::onControlPressed()
{
    if (_walletEditor) {
        _control->showMenu();
    }
}

void WalletControlWidget::onDelete()
{
    int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the wallet '%1'?", _walletName),QString(),KStandardGuiItem::del());
    if (rc != KMessageBox::Continue) {
        return;
    }
    rc = KWallet::Wallet::deleteWallet(_walletName);
    if (rc != 0) {
        KMessageBox::sorry(this, i18n("Unable to delete the wallet. Error code was %1.", rc));
    }
}
