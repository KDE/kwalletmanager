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
#include <kiconview.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <ksystemtray.h>
#include <kwallet.h>

#include <qaccel.h>
#include <qguardedptr.h>
#include <qptrstack.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qtooltip.h>

KWalletManager::KWalletManager(QWidget *parent, const char *name, WFlags f)
: KMainWindow(parent, name, f), DCOPObject("KWalletManager") {
	KGlobal::dirs()->addResourceType("kwallet", "share/apps/kwallet");
	_kwalletdLaunch = false;
	QAccel *accel = new QAccel(this, "kwalletmanager");

	KApplication::dcopClient()->setQtBridgeEnabled(false);
	_shuttingDown = false;
	KConfig cfg("kwalletrc"); // not sure why this setting isn't in kwalletmanagerrc...
	KConfigGroup walletConfigGroup(&cfg, "Wallet");
	_dcopRef = 0L;
	if (walletConfigGroup.readBoolEntry("Launch Manager", true)) {
		_tray = new KSystemTray(this, "kwalletmanager tray");
		_tray->setPixmap(loadSystemTrayIcon("wallet_closed"));
		QToolTip::add(_tray, i18n("KDE Wallet: No wallets open."));
		connect(_tray, SIGNAL(quitSelected()), SLOT(shuttingDown()));
		QStringList wl = KWallet::Wallet::walletList();
		bool isOpen = false;
		for (QStringList::Iterator it = wl.begin(); it != wl.end(); ++it) {
			if (KWallet::Wallet::isOpen(*it)) {
				_tray->setPixmap(loadSystemTrayIcon("wallet_open"));
				QToolTip::remove(_tray);
				QToolTip::add(_tray, i18n("KDE Wallet: A wallet is open."));
				isOpen = true;
				break;
			}
		}
		if (!isOpen && kapp->isRestored()) {
			delete _tray;
			_tray = 0L;
			QTimer::singleShot( 0, kapp, SLOT( quit()));
			return;
		}
	} else {
		_tray = 0L;
	}

	_iconView = new KWalletIconView(this, "kwalletmanager icon view");
	connect(_iconView, SIGNAL(executed(QIconViewItem*)), this, SLOT(openWallet(QIconViewItem*)));
	connect(_iconView, SIGNAL(contextMenuRequested(QIconViewItem*, const QPoint&)), this, SLOT(contextMenu(QIconViewItem*, const QPoint&)));

	updateWalletDisplay();
	setCentralWidget(_iconView);
	_iconView->setMinimumSize(320, 200);

	_dcopRef = new DCOPRef("kded", "kwalletd");
	_dcopRef->dcopClient()->setNotifications(true);
	connect(_dcopRef->dcopClient(),
		SIGNAL(applicationRemoved(const QCString&)),
		this,
		SLOT(possiblyRescan(const QCString&)));
	connect(_dcopRef->dcopClient(),
		SIGNAL(applicationRegistered(const QCString&)),
		this,
		SLOT(possiblyRescan(const QCString&)));

	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "allWalletsClosed()", "allWalletsClosed()", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletClosed(QString)", "updateWalletDisplay()", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletOpened(QString)", "aWalletWasOpened()", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletDeleted(QString)", "updateWalletDisplay()", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletListDirty()", "updateWalletDisplay()", false);

	// FIXME: slight race - a wallet can open, then we get launched, but the
	//        wallet closes before we are done opening.  We will then stay
	//        open.  Must check that a wallet is still open here.

	new KAction(i18n("&New Wallet..."), "kwalletmanager", 0, this,
			SLOT(createWallet()), actionCollection(),
			"wallet_create");
	KAction *act = new KAction(i18n("Configure &Wallet..."), "configure",
			0, this, SLOT(setupWallet()), actionCollection(),
			"wallet_settings");
	if (_tray) {
		act->plug(_tray->contextMenu());
	}
	act = new KAction(i18n("Close &All Wallets"), 0, 0, this,
			SLOT(closeAllWallets()), actionCollection(),
			"close_all_wallets");
	if (_tray) {
		act->plug(_tray->contextMenu());
	}
	KStdAction::quit(this, SLOT(shuttingDown()), actionCollection());
	KStdAction::keyBindings(guiFactory(), SLOT(configureShortcuts()),
actionCollection());

	createGUI("kwalletmanager.rc");
	accel->connectItem(accel->insertItem(Key_Return), this, SLOT(openWallet()));
	accel->connectItem(accel->insertItem(Key_Delete), this, SLOT(deleteWallet()));

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
		QToolTip::remove(_tray);
		QToolTip::add(_tray, i18n("KDE Wallet: A wallet is open."));
	}
	updateWalletDisplay();
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
			new KWalletItem(_iconView, *i);
		} else {
			// FIXME: See if icon needs to be updated
		}
	}
}


void KWalletManager::contextMenu(QIconViewItem *item, const QPoint& pos) {
	if (item) {
		QGuardedPtr<KWalletPopup> popupMenu = new KWalletPopup(item->text(), this);
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
	int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the wallet '%1'?").arg(walletName),"",KStdGuiItem::del());
	if (rc != KMessageBox::Continue) {
		return;
	}
	rc = KWallet::Wallet::deleteWallet(walletName);
	if (rc != 0) {
		KMessageBox::sorry(this, i18n("Unable to delete the wallet. Error code was %1.").arg(rc));
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
				KMessageBox::sorry(this, i18n("Unable to force the wallet closed. Error code was %1.").arg(rc));
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
		KMessageBox::sorry(this, i18n("Error opening wallet %1.").arg(path));
		delete we;
	}
}


void KWalletManager::openWallet() {
	QIconViewItem *item = _iconView->currentItem();
	openWallet(item);
}

void KWalletManager::deleteWallet() {
	QIconViewItem *item = _iconView->currentItem();
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
	if (_tray) {
		_tray->setPixmap(loadSystemTrayIcon("wallet_closed"));
		QToolTip::remove(_tray);
		QToolTip::add(_tray, i18n("KDE Wallet: No wallets open."));
	}
	possiblyQuit();
}


void KWalletManager::possiblyQuit() {
	KConfig cfg("kwalletrc");
	cfg.setGroup("Wallet");
	if (_windows.isEmpty() &&
			!isVisible() &&
			!cfg.readBoolEntry("Leave Manager Open", false) &&
			_kwalletdLaunch) {
		kapp->quit();
	}
}


void KWalletManager::editorClosed(KMainWindow* e) {
	_windows.remove(e);
}


void KWalletManager::possiblyRescan(const QCString& app) {
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
	KApplication::startServiceByDesktopName("kwallet_config");
}


void KWalletManager::closeAllWallets() {
	_dcopRef->call("closeAllWallets");
}


QPixmap KWalletManager::loadSystemTrayIcon(const QString &icon) {
#if KDE_IS_VERSION(3, 1, 90)
	return KSystemTray::loadIcon(icon);
#else
	KConfig *appCfg = kapp->config();
	KConfigGroupSaver configSaver(appCfg, "System Tray");
	int iconWidth = appCfg->readNumEntry("systrayIconWidth", 22);
	return kapp->iconLoader()->loadIcon( icon, KIcon::Panel, iconWidth );
#endif
}


#include "kwalletmanager.moc"
