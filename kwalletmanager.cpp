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


#include "kwalletmanager.h"
#include "kwalletpopup.h"
#include "kwalleteditor.h"

#include <dcopref.h>
#include <dcopclient.h>
#include <kiconview.h>
#include <ksystemtray.h>
#include <kwallet.h>
#include <kstdaction.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <klocale.h>

#include <qptrstack.h>


KWalletManager::KWalletManager(QWidget *parent, const char *name, WFlags f)
: KMainWindow(parent, name, f), DCOPObject("KWalletManager") {
	_tray = new KSystemTray(this, "kwalletmanager tray");
	_tray->show();

	_iconView = new KIconView(this, "kwalletmanager icon view");
	connect(_iconView, SIGNAL(executed(QIconViewItem*)), this, SLOT(openWallet(QIconViewItem*)));
	connect(_iconView, SIGNAL(contextMenuRequested(QIconViewItem*, const QPoint&)), this, SLOT(contextMenu(QIconViewItem*, const QPoint&)));
	updateWalletDisplay();
	setCentralWidget(_iconView);
	_iconView->arrangeItemsInGrid();

	_dcopRef = new DCOPRef("kded", "kwalletd");

        connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "allWalletsClosed()", "allWalletsClosed()", false);
        connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletClosed(const QString&)", "updateWalletDisplay()", false);
        connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletOpened(const QString&)", "updateWalletDisplay()", false);
        connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletDeleted(const QString&)", "updateWalletDisplay()", false);

	// FIXME: slight race - a wallet can open, then we get launched, but the
	//        wallet closes before we are done opening.  We will then stay
	//        open.  Must check that a wallet is still open here.

	KStdAction::quit(qApp, SLOT(quit()), actionCollection());
	setXMLFile("kwalletmanager.rc");
	createGUI();
}


KWalletManager::~KWalletManager() {
	_tray = 0L;
	delete _dcopRef;
	_dcopRef = 0L;
}


void KWalletManager::updateWalletDisplay() {
QStringList wl = KWallet::Wallet::walletList();
QPtrStack<QIconViewItem> trash;

	for (QIconViewItem *item = _iconView->firstItem(); item; item = item->nextItem()) {
		if (!wl.contains(item->text())) {
			trash.push(item);
		}
	}

	trash.setAutoDelete(true);
	trash.clear();

	for (QStringList::Iterator i = wl.begin(); i != wl.end(); ++i) {
		if (!_iconView->findItem(*i)) {
			// FIXME: if KWallet::Wallet::isOpen(*i) then show
			//        a different icon!
			new QIconViewItem(_iconView, *i);
		} else {
			// FIXME: See if icon needs to be updated
		}
	}
}


void KWalletManager::contextMenu(QIconViewItem *item, const QPoint& pos) {
	if (item) {
		KWalletPopup *p = new KWalletPopup(item->text(), this);
		connect(p, SIGNAL(walletOpened(const QString&)), this, SLOT(openWallet(const QString&)));
		connect(p, SIGNAL(walletClosed(const QString&)), this, SLOT(closeWallet(const QString&)));
		connect(p, SIGNAL(walletDeleted(const QString&)), this, SLOT(deleteWallet(const QString&)));
		p->popup(pos);
	}
}


void KWalletManager::deleteWallet(const QString& walletName) {
	int rc = KWallet::Wallet::deleteWallet(walletName);
	if (rc != 0) {
		KMessageBox::sorry(this, i18n("Unable to delete the wallet.  Error code was %1.").arg(rc));
	}
	updateWalletDisplay();
}


void KWalletManager::closeWallet(const QString& walletName) {
	int rc = KWallet::Wallet::closeWallet(walletName, false);
	if (rc != 0) {
		rc = KMessageBox::warningYesNo(this, i18n("Unable to close wallet cleanly.  It is probably in use by other applications.  Do you wish to force it closed?"));
		if (rc == KMessageBox::Yes) {
			rc = KWallet::Wallet::closeWallet(walletName, true);
			if (rc != 0) {
				KMessageBox::sorry(this, i18n("Unable to force the wallet closed.  Error code was %1.").arg(rc));
			}
		}
	}

	updateWalletDisplay();
}


void KWalletManager::openWallet(const QString& walletName) {
	KWalletEditor *we = new KWalletEditor(walletName, this, "Wallet Editor");
	if (we->isOpen()) {
		connect(we, SIGNAL(editorClosed(KMainWindow*)),
			this, SLOT(editorClosed(KMainWindow*)));
		we->show();
		_windows.append(we);
	} else {
		KMessageBox::sorry(this, i18n("Error opening wallet %1.").arg(walletName));
		delete we;
	}
}


void KWalletManager::openWallet(QIconViewItem *item) {
	if (item) {
		openWallet(item->text());
	}
}


void KWalletManager::allWalletsClosed() {
	possiblyQuit();
}


void KWalletManager::possiblyQuit() {
	if (_windows.isEmpty() && !isVisible()) {
		close();
	}
}


void KWalletManager::editorClosed(KMainWindow* e) {
	_windows.remove(e);
}

// TODO: - ability to see who is using which wallets?
//       - icons
//       - statusbar
//       - create wallet
//       - copy wallet (DnD)
//       - move wallet (DnD)
//       - drop of wallet into the app

#include "kwalletmanager.moc"

