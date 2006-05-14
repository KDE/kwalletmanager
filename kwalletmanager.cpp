/*
   Copyright (C) 2003,2004 George Staikos <staikos@kde.org>

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


#include "kwalletmanager.h"
#include "kwalletpopup.h"
#include "kwalleteditor.h"
#include "allyourbase.h"

#include <dcopclient.h>
#include <dcopref.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <k3iconview.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <ksystemtray.h>
#include <kwallet.h>
#include <kxmlguifactory.h>
#include <q3accel.h>
#include <qpointer.h>
#include <q3ptrstack.h>
#include <QRegExp>
#include <QToolTip>
//Added by qt3to4:
#include <QPixmap>
#include <ktoolinvocation.h>

KWalletManager::KWalletManager(QWidget *parent, const char *name, Qt::WFlags f)
: KMainWindow(parent, name, f), DCOPObject("KWalletManager") {
	KGlobal::dirs()->addResourceType("kwallet", "share/apps/kwallet");
	_kwalletdLaunch = false;
	Q3Accel *accel = new Q3Accel(this, "kwalletmanager");

	KApplication::dcopClient()->setQtBridgeEnabled(false);
	_shuttingDown = false;
	KConfig cfg("kwalletrc"); // not sure why this setting isn't in kwalletmanagerrc...
	KConfigGroup walletConfigGroup(&cfg, "Wallet");
	if (walletConfigGroup.readEntry("Launch Manager", true)) {
		_tray = new KSystemTray(this);
		_tray->setObjectName("kwalletmanager tray");
		_tray->setPixmap(loadSystemTrayIcon("wallet_closed"));
		_tray->setToolTip( i18n("KDE Wallet: No wallets open."));
		connect(_tray,SIGNAL(quitSelected()),SLOT(shuttingDown()));
		QStringList wl = KWallet::Wallet::walletList();
		for (QStringList::Iterator it = wl.begin(); it != wl.end(); ++it) {
			if (KWallet::Wallet::isOpen(*it)) {
				_tray->setPixmap(loadSystemTrayIcon("wallet_open"));
				_tray->setToolTip( i18n("KDE Wallet: A wallet is open."));
				break;
			}
		}
	} else {
		_tray = 0;
	}

	_iconView = new KWalletIconView(this, "kwalletmanager icon view");
	connect(_iconView, SIGNAL(executed(Q3IconViewItem*)), this, SLOT(openWallet(Q3IconViewItem*)));
	connect(_iconView, SIGNAL(contextMenuRequested(Q3IconViewItem*, const QPoint&)), this, SLOT(contextMenu(Q3IconViewItem*, const QPoint&)));

	updateWalletDisplay();
	setCentralWidget(_iconView);
	_iconView->setMinimumSize(320, 200);

	_dcopRef = new DCOPRef("kded", "kwalletd");
	_dcopRef->dcopClient()->setNotifications(true);
	connect(_dcopRef->dcopClient(),
		SIGNAL(applicationRemoved(const QByteArray&)),
		this,
		SLOT(possiblyRescan(const QByteArray&)));
	connect(_dcopRef->dcopClient(),
		SIGNAL(applicationRegistered(const QByteArray&)),
		this,
		SLOT(possiblyRescan(const QByteArray&)));

	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "allWalletsClosed()", "allWalletsClosed()", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletClosed(QString)", "updateWalletDisplay()", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletOpened(QString)", "aWalletWasOpened()", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletDeleted(QString)", "updateWalletDisplay()", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletListDirty()", "updateWalletDisplay()", false);

	// FIXME: slight race - a wallet can open, then we get launched, but the
	//        wallet closes before we are done opening.  We will then stay
	//        open.  Must check that a wallet is still open here.

	KAction *action = new KAction(KIcon("kwalletmanager"), i18n("&New Wallet..."), actionCollection(), "wallet_create");
	connect(action, SIGNAL(triggered(bool) ), SLOT(createWallet()));
	KAction *act = new KAction(KIcon("configure"), i18n("Configure &Wallet..."), actionCollection(), "wallet_settings");
	connect(act, SIGNAL(triggered(bool) ), SLOT(setupWallet()));
	if (_tray) {
		_tray->contextMenu()->addAction( act );
	}
	act = new KAction(i18n("Close &All Wallets"), actionCollection(), "close_all_wallets");
	connect(act, SIGNAL(triggered(bool) ), SLOT(closeAllWallets()));
	if (_tray) {
		_tray->contextMenu()->addAction( act );
	}
	KStdAction::quit(this, SLOT(shuttingDown()), actionCollection());
	KStdAction::keyBindings(guiFactory(), SLOT(configureShortcuts()),
actionCollection());

	createGUI("kwalletmanager.rc");
	accel->connectItem(accel->insertItem(Qt::Key_Return), this, SLOT(openWallet()));
	accel->connectItem(accel->insertItem(Qt::Key_Delete), this, SLOT(deleteWallet()));

	if (_tray) {
		_tray->show();
	} else {
		show();
	}

	kapp->setName("kwallet"); // hack to fix docs
}


KWalletManager::~KWalletManager() {
	_tray = 0L;
	delete _dcopRef;
	_dcopRef = 0L;
}


void KWalletManager::kwalletdLaunch() {
	_kwalletdLaunch = true;
}


bool KWalletManager::queryClose() {
	if (!_shuttingDown && !kapp->sessionSaving()) {
		if (!_tray) {
			kapp->quit();
		} else {
			hide();
		}
		return false;
	}
	return true;
}


void KWalletManager::aWalletWasOpened() {
	if (_tray) {
		_tray->setPixmap(loadSystemTrayIcon("wallet_open"));
		_tray->setToolTip( i18n("KDE Wallet: A wallet is open."));
	}
	updateWalletDisplay();
}


void KWalletManager::updateWalletDisplay() {
QStringList wl = KWallet::Wallet::walletList();
Q3PtrStack<Q3IconViewItem> trash;

	for (Q3IconViewItem *item = _iconView->firstItem(); item; item = item->nextItem()) {
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
			new KWalletItem(_iconView, *i);
		} else {
			// FIXME: See if icon needs to be updated
		}
	}
}


void KWalletManager::contextMenu(Q3IconViewItem *item, const QPoint& pos) {
	if (item) {
		QPointer<KWalletPopup> popupMenu = new KWalletPopup(item->text(), this);
		connect(popupMenu, SIGNAL(walletOpened(const QString&)), this, SLOT(openWallet(const QString&)));
		connect(popupMenu, SIGNAL(walletClosed(const QString&)), this, SLOT(closeWallet(const QString&)));
		connect(popupMenu, SIGNAL(walletDeleted(const QString&)), this, SLOT(deleteWallet(const QString&)));
		connect(popupMenu, SIGNAL(walletChangePassword(const QString&)), this, SLOT(changeWalletPassword(const QString&)));
		connect(popupMenu, SIGNAL(walletCreated()), this, SLOT(createWallet()));
		popupMenu->exec(pos);
		delete popupMenu;
	}
}


void KWalletManager::deleteWallet(const QString& walletName) {
	int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the wallet '%1'?", walletName),"",KStdGuiItem::del());
	if (rc != KMessageBox::Continue) {
		return;
	}
	rc = KWallet::Wallet::deleteWallet(walletName);
	if (rc != 0) {
		KMessageBox::sorry(this, i18n("Unable to delete the wallet. Error code was %1.", rc));
	}
	updateWalletDisplay();
}


void KWalletManager::closeWallet(const QString& walletName) {
	int rc = KWallet::Wallet::closeWallet(walletName, false);
	if (rc != 0) {
		rc = KMessageBox::warningYesNo(this, i18n("Unable to close wallet cleanly. It is probably in use by other applications. Do you wish to force it closed?"), QString::null, i18n("Force Closure"), i18n("Do Not Force"));
		if (rc == KMessageBox::Yes) {
			rc = KWallet::Wallet::closeWallet(walletName, true);
			if (rc != 0) {
				KMessageBox::sorry(this, i18n("Unable to force the wallet closed. Error code was %1.", rc));
			}
		}
	}

	updateWalletDisplay();
}


void KWalletManager::changeWalletPassword(const QString& walletName) {
	KWallet::Wallet::changePassword(walletName);
}


void KWalletManager::openWalletFile(const QString& path) {
	KWalletEditor *we = new KWalletEditor(path, true, this, "Wallet Editor");
	if (we->isOpen()) {
		connect(we, SIGNAL(editorClosed(KMainWindow*)),
			this, SLOT(editorClosed(KMainWindow*)));
		we->show();
	} else {
		KMessageBox::sorry(this, i18n("Error opening wallet %1.", path));
		delete we;
	}
}


void KWalletManager::openWallet() {
	Q3IconViewItem *item = _iconView->currentItem();
	openWallet(item);
}

void KWalletManager::deleteWallet() {
	Q3IconViewItem *item = _iconView->currentItem();
	if (item) {
		deleteWallet(item->text());
	}
}


void KWalletManager::openWallet(const QString& walletName) {
	openWallet(walletName, false);
}


void KWalletManager::openWallet(const QString& walletName, bool newWallet) {
	// Don't allow a wallet to open in two windows
	for (KMainWindow *w = _windows.first(); w; w = _windows.next()) {
		KWalletEditor *e = static_cast<KWalletEditor*>(w);
		if (e->isOpen() && e->_walletName == walletName) {
			w->raise();
			return;
		}
	}

	KWalletEditor *we = new KWalletEditor(walletName, false, this, "Wallet Editor");
	we->setNewWallet(newWallet);
	if (we->isOpen()) {
		connect(we, SIGNAL(editorClosed(KMainWindow*)),
			this, SLOT(editorClosed(KMainWindow*)));
		we->show();
		_windows.append(we);
	} else if (!newWallet) {
		KMessageBox::sorry(this, i18n("Error opening wallet %1.", walletName));
		delete we;
	}
}


void KWalletManager::openWallet(Q3IconViewItem *item) {
	if (item) {
		openWallet(item->text());
	}
}


void KWalletManager::allWalletsClosed() {
	if (_tray) {
		_tray->setPixmap(loadSystemTrayIcon("wallet_closed"));
		_tray->setToolTip( i18n("KDE Wallet: No wallets open."));
	}
	possiblyQuit();
}


void KWalletManager::possiblyQuit() {
	KConfig cfg("kwalletrc");
	cfg.setGroup("Wallet");
	if (_windows.isEmpty() &&
			!isVisible() &&
			!cfg.readEntry("Leave Manager Open", false) &&
			_kwalletdLaunch) {
		kapp->quit();
	}
}


void KWalletManager::editorClosed(KMainWindow* e) {
	_windows.remove(e);
}


void KWalletManager::possiblyRescan(const QByteArray& app) {
	if (app == "kded") {
		updateWalletDisplay();
	}
}


void KWalletManager::createWallet() {
	QString n;
	bool ok;
	// FIXME: support international names
	QRegExp regexp("^[A-Za-z0-9]+[A-Za-z0-9_\\s\\-]*$");
	QString txt = i18n("Please choose a name for the new wallet:");

	if (!KWallet::Wallet::isEnabled()) {
		// FIXME: KMessageBox::warningYesNo(this, i1_8n("KWallet is not enabled.  Do you want to enable it?"), QString::null, i18n("Enable"), i18n("Keep Disabled"));
		return;
	}

	do {
		n = KInputDialog::getText(i18n("New Wallet"),
				txt,
				QString::null,
				&ok,
				this);

		if (!ok) {
			return;
		}

		if (_iconView->findItem(n)) {
			int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that wallet already exists. Try a new name?"), QString::null, i18n("Try New"), i18n("Do Not Try"));
			if (rc == KMessageBox::Yes) {
				continue;
			}
			n = QString::null;
		} else if (regexp.exactMatch(n)) {
			break;
		} else {
			txt = i18n("Please choose a name that contains only alphanumeric characters:");
		}
	} while (true);

	// Small race here - the wallet could be created on us already.
	if (!n.isEmpty()) {
		openWallet(n, true);
	}
}


void KWalletManager::shuttingDown() {
	_shuttingDown = true;
	kapp->quit();
}


void KWalletManager::setupWallet() {
	KToolInvocation::startServiceByDesktopName("kwallet_config");
}


void KWalletManager::closeAllWallets() {
	_dcopRef->call("closeAllWallets");
}


QPixmap KWalletManager::loadSystemTrayIcon(const QString &icon) {
	return KSystemTray::loadIcon(icon);
}


#include "kwalletmanager.moc"
