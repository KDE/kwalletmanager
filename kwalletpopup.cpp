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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "kwalletpopup.h"

#include <kaction.h>
#include <kdebug.h>
#include <kiconview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kwallet.h>
#include <kstdguiitem.h>

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

	QStringList ul = KWallet::Wallet::users(wallet);
	if (!ul.isEmpty()) {
		KPopupMenu *pm = new KPopupMenu(this, "Disconnect Apps");
		int id = 7000;
		for (QStringList::Iterator it = ul.begin(); it != ul.end(); ++it) {
			_appMap[id] = *it;
			pm->insertItem(*it, this, SLOT(disconnectApp(int)), 0, id);
			pm->setItemParameter(id, id);
			id++;
		}

		insertItem(i18n("Disconnec&t"), pm);
	}

	act = KStdAction::close( this,
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


void KWalletPopup::disconnectApp(int id) {
	KWallet::Wallet::disconnectApplication(_walletName, _appMap[id].latin1());
}

#include "kwalletpopup.moc"

