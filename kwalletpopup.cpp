/*
   Copyright (C) 2003 George Staikos <staikos@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include "kwalletpopup.h"

#include <kiconview.h>
#include <kwallet.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>


KWalletPopup::KWalletPopup(const QString& wallet, QWidget *parent, const char *name)
: KPopupMenu(parent, name), _walletName(wallet) {
	insertTitle(wallet);
	KActionCollection *ac = new KActionCollection(this, "kwallet context actions");
	KAction *act;

	act = new KAction(i18n("&New Wallet..."), 0, 0, this,
			SLOT(createWallet()), ac, "wallet_create");
	act->plug(this);

	act = new KAction(i18n("&Open..."), 0, Key_Return, this,
			SLOT(openWallet()), ac, "wallet_open");
	act->plug(this);

	act = new KAction(i18n("Change &Password..."), 0, 0, this,
			SLOT(changeWalletPassword()), ac, "wallet_password");
	act->plug(this);

	act = new KAction(i18n("&Close"), 0, 0, this,
			SLOT(closeWallet()), ac, "wallet_close");
	// FIXME: let's track this inside the manager so we don't need a dcop
	//        roundtrip here.
	act->setEnabled(KWallet::Wallet::isOpen(wallet));
	act->plug(this);

	act = new KAction(i18n("&Delete"), 0, Key_Delete, this,
			SLOT(deleteWallet()), ac, "wallet_delete");
	act->plug(this);
}


KWalletPopup::~KWalletPopup() {
}


void KWalletPopup::openWallet() {
	emit walletOpened(_walletName);
}


void KWalletPopup::deleteWallet() {
	emit walletDeleted(_walletName);
}


void KWalletPopup::closeWallet() {
	emit walletClosed(_walletName);
}


void KWalletPopup::changeWalletPassword() {
	emit walletChangePassword(_walletName);
}


void KWalletPopup::createWallet() {
	emit walletCreated();
}


#include "kwalletpopup.moc"

