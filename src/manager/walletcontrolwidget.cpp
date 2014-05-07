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
#include "applicationsmanager.h"

#include <QPropertyAnimation>
#include <QTimer>
#include <QFrame>
#include <QToolButton>
#include <qevent.h>
#include <kwallet.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <KTabWidget>
#include <kdebug.h>

WalletControlWidget::WalletControlWidget(QWidget* parent, const QString& walletName):
    QWidget(parent),
    _walletName(walletName),
    _wallet(0),
    _walletEditor(0),
    _applicationsManager(0)
{
    setupUi(this);
    onSetupWidget();

    QTimer::singleShot(1, this, SLOT(onSetupWidget()));
}

bool WalletControlWidget::openWallet()
{
    bool result = false;
    if (_wallet && _wallet->isOpen()) {
        result = true; // already opened
    } else {
        _wallet = KWallet::Wallet::openWallet(_walletName, effectiveWinId());
        result = _wallet != 0;
        onSetupWidget();
    }
    return result;
}

void WalletControlWidget::onSetupWidget()
{
    if (KWallet::Wallet::isOpen(_walletName)) {
        if (0 == _wallet) {
            _wallet = KWallet::Wallet::openWallet(_walletName, effectiveWinId());
            if (0 == _wallet) {
                kDebug() << "Weird situation: wallet could not be opened when setting-up the widget.";
            }
        }
    }

    if (_wallet) {
        connect(_wallet, SIGNAL(walletClosed()), this, SLOT(onWalletClosed()));
        _openClose->setText(i18n("&Close"));

        if (0 == _walletEditor) {
            _walletEditor = new KWalletEditor(_editorFrame);
            _editorFrameLayout->addWidget(_walletEditor);
            _walletEditor->setVisible(true);
        }
        _walletEditor->setWallet(_wallet);

        if (0 == _applicationsManager) {
            _applicationsManager = new ApplicationsManager(_applicationsFrame);
            _applicationsFrameLayout->addWidget(_applicationsManager);
            _applicationsManager->setVisible(true);
        }
        _applicationsManager->setWallet(_wallet);

        _changePassword->setEnabled(true);
        _stateLabel->setText(i18nc("the 'kdewallet' is currently open (e.g. %1 will be replaced with current wallet name)", "The '%1' wallet is currently open", _walletName));
        _tabs->setTabIcon(0, QIcon::fromTheme( QLatin1String("wallet-open")).pixmap(16));
    } else {
        _openClose->setText(i18n("&Open..."));

        if (_walletEditor) {
            _walletEditor->setVisible(false);
            delete _walletEditor, _walletEditor =0;
        }

        if (_applicationsManager) {
            _applicationsManager->setVisible(false);
            delete _applicationsManager, _applicationsManager = 0;
        }
        _changePassword->setEnabled(false);
        _stateLabel->setText(i18n("The wallet is currently closed"));
        _tabs->setTabIcon(0, QIcon::fromTheme( QLatin1String("wallet-closed")).pixmap(16));
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
    } else {
        _wallet = KWallet::Wallet::openWallet(_walletName, window()->winId());
    }
    onSetupWidget();
}

void WalletControlWidget::onWalletClosed()
{
    _wallet = 0;
    onSetupWidget();
}

void WalletControlWidget::updateWalletDisplay()
{
//     QList<QAction*> existingActions = _disconnect->actions();
//     QList<QAction*>::const_iterator i = existingActions.constBegin();
//     QList<QAction*>::const_iterator ie = existingActions.constEnd();
//     for ( ; i != ie; i++ ) {
//         _disconnect->removeAction(*i);
//     }
// 
}

void WalletControlWidget::onDisconnectApplication()
{
    QAction *a = qobject_cast<QAction *>(sender());
    Q_ASSERT(a);
    if (a)  {
        KWallet::Wallet::disconnectApplication(_walletName, a->data().toString());
    }
}

void WalletControlWidget::onChangePassword()
{
    KWallet::Wallet::changePassword(_walletName, effectiveWinId());
}

void WalletControlWidget::hideEvent(QHideEvent* )
{
}

void WalletControlWidget::showEvent(QShowEvent* ev)
{
}

#include "walletcontrolwidget.moc"
