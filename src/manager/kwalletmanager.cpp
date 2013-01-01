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
#include "kwallet_interface.h"

#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kstandardaction.h>
#include <kstatusnotifieritem.h>
#include <kwallet.h>
#include <kxmlguifactory.h>
#include <QPointer>
#include <QRegExp>

#include <QRegExpValidator>
#include <QTimer>
#include <ktoolinvocation.h>
#include <kicon.h>
#include <kactioncollection.h>
#include <kconfiggroup.h>

KWalletManager::KWalletManager(QWidget *parent, const char *name, Qt::WFlags f)
    : KXmlGuiWindow(parent, f)
{
    setObjectName(QLatin1String( name ) );
    QDBusConnection::sessionBus().registerObject(QLatin1String( "/KWalletManager" ), this, QDBusConnection::ExportScriptableSlots);
	KGlobal::dirs()->addResourceType("kwallet", 0, QLatin1String( "share/apps/kwallet" ));
	_kwalletdLaunch = false;
	_shuttingDown = false;
	m_kwalletdModule = 0;
	KConfig cfg( QLatin1String( "kwalletrc" )); // not sure why this setting isn't in kwalletmanagerrc...
	KConfigGroup walletConfigGroup(&cfg, "Wallet");
	if (walletConfigGroup.readEntry("Launch Manager", true)) {
		_tray = new KStatusNotifierItem(this);
		_tray->setObjectName( QLatin1String("kwalletmanager tray" ));
        _tray->setCategory( KStatusNotifierItem::SystemServices );
        _tray->setStatus( KStatusNotifierItem::Passive );
		_tray->setIconByName(QLatin1String( "wallet-closed" ));
		_tray->setToolTip( QLatin1String( "wallet-closed" ), i18n("KDE Wallet"), i18n("No wallets open."));
		//connect(_tray, SIGNAL(quitSelected()), SLOT(shuttingDown()));
		const QStringList wl = KWallet::Wallet::walletList();
		bool isOpen = false;
		for (QStringList::ConstIterator it = wl.begin(); it != wl.end(); ++it) {
			if (KWallet::Wallet::isOpen(*it)) {
				_tray->setIconByName(QLatin1String( "wallet-open" ));
				_tray->setToolTip( QLatin1String( "wallet-open" ), i18n("KDE Wallet"), i18n("A wallet is open."));
				isOpen = true;
				break;
			}
		}
		if (!isOpen && qApp->isSessionRestored()) {
			delete _tray;
			_tray = 0;
			QTimer::singleShot( 0, qApp, SLOT(quit()));
			return;
		}
	} else {
		_tray = 0;
	}

	_iconView = new KWalletIconView(this);
	_iconView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(_iconView, SIGNAL(executed(QListWidgetItem*)), this, SLOT(openWallet(QListWidgetItem*)));
	connect(_iconView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));

	updateWalletDisplay();
	setCentralWidget(_iconView);
	setAutoSaveSettings(QLatin1String("MainWindow"), true);
	_iconView->setMinimumSize(320, 200);

        m_kwalletdModule = new org::kde::KWallet(QLatin1String( "org.kde.kwalletd" ), QLatin1String( "/modules/kwalletd" ), QDBusConnection::sessionBus());
        connect(QDBusConnection::sessionBus().interface(),
                SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                this,
                SLOT(possiblyRescan(QString,QString,QString)));
        connect( m_kwalletdModule, SIGNAL(allWalletsClosed()),
                 this, SLOT(allWalletsClosed()) );
        connect( m_kwalletdModule, SIGNAL(walletClosed(QString)),
                 this, SLOT(updateWalletDisplay()) );
        connect( m_kwalletdModule, SIGNAL(walletOpened(QString)),
                 this, SLOT(aWalletWasOpened()) );
        connect( m_kwalletdModule, SIGNAL(walletDeleted(QString)),
                 this, SLOT(updateWalletDisplay()) );
        connect( m_kwalletdModule, SIGNAL(walletListDirty()),
                 this, SLOT(updateWalletDisplay()) );
	// FIXME: slight race - a wallet can open, then we get launched, but the
	//        wallet closes before we are done opening.  We will then stay
	//        open.  Must check that a wallet is still open here.

	QAction *action = actionCollection()->addAction(QLatin1String( "wallet_create" ));
	action->setText(i18n("&New Wallet..."));
	action->setIcon(KIcon( QLatin1String( "kwalletmanager" )));
	connect(action, SIGNAL(triggered()), SLOT(createWallet()));
	QAction *act = actionCollection()->addAction(QLatin1String( "wallet_settings" ));
	act->setText(i18n("Configure &Wallet..."));
	act->setIcon(KIcon( QLatin1String( "configure" )));

	connect(act, SIGNAL(triggered()), SLOT(setupWallet()));
	if (_tray) {
		_tray->contextMenu()->addAction( act );
	}
	act = actionCollection()->addAction(QLatin1String( "close_all_wallets" ));
	act->setText(i18n("Close &All Wallets"));
	connect(act, SIGNAL(triggered()), SLOT(closeAllWallets()));
	if (_tray) {
		_tray->contextMenu()->addAction( act );
	}
	KStandardAction::quit(this, SLOT(shuttingDown()), actionCollection());
	KStandardAction::keyBindings(guiFactory(), SLOT(configureShortcuts()),
actionCollection());

	createGUI( QLatin1String( "kwalletmanager.rc" ));

        KAction* openWalletAction = new KAction(i18n("Open Wallet"), actionCollection());
        openWalletAction->setShortcut(Qt::Key_Return);
        connect(openWalletAction, SIGNAL(triggered()), this, SLOT(openWallet()));

        KAction* deleteWalletAction = new KAction(i18n("Delete Wallet"), actionCollection());
        deleteWalletAction->setShortcut(Qt::Key_Delete);
        connect(deleteWalletAction, SIGNAL(triggered()), this, SLOT(deleteWallet()));

	if (_tray) {
//		_tray->show();
	} else {
		show();
	}

	qApp->setObjectName( QLatin1String("kwallet" )); // hack to fix docs
}


KWalletManager::~KWalletManager() {
	_tray = 0L;
        delete m_kwalletdModule;
        m_kwalletdModule=0L;
}


void KWalletManager::kwalletdLaunch() {
	_kwalletdLaunch = true;
}


bool KWalletManager::queryClose() {
	if (!_shuttingDown && !kapp->sessionSaving()) {
		if (!_tray) {
			qApp->quit();
		} else {
			hide();
		}
		return false;
	}
	return true;
}


void KWalletManager::aWalletWasOpened() {
	if (_tray) {
		_tray->setIconByName(QLatin1String( "wallet-open" ));
		_tray->setToolTip( QLatin1String( "wallet-open" ), i18n("KDE Wallet"), i18n("A wallet is open."));
		_tray->setStatus(KStatusNotifierItem::Active);
	}
	updateWalletDisplay();
}


void KWalletManager::updateWalletDisplay() {
    const QStringList wl = KWallet::Wallet::walletList();
    QStack<QListWidgetItem *> trash;

	for (int i = 0; i < _iconView->count(); ++i) {
		QListWidgetItem *item = _iconView->item(i);
		if (!wl.contains(item->text())) {
			trash.push(item);
		}
	}

	qDeleteAll(trash);
	trash.clear();

	for (QStringList::const_iterator i = wl.begin(); i != wl.end(); ++i) {
		const QList<QListWidgetItem *> items = _iconView->findItems(*i, Qt::MatchFixedString | Qt::MatchCaseSensitive);
		KWalletItem *wi = 0;
		if (items.isEmpty()) {
			wi = new KWalletItem(_iconView, *i);
		} else {
			wi = dynamic_cast<KWalletItem*>(items[0]);
		}
		if (wi) {
			wi->setOpen(KWallet::Wallet::isOpen(*i));
		}
	}
}


void KWalletManager::contextMenu(const QPoint& pos) {
	QListWidgetItem *item = _iconView->itemAt(pos);
	if (item) {
		QPointer<KWalletPopup> popupMenu = new KWalletPopup(item->text(), this);
		connect(popupMenu, SIGNAL(walletOpened(QString)), this, SLOT(openWallet(QString)));
		connect(popupMenu, SIGNAL(walletClosed(QString)), this, SLOT(closeWallet(QString)));
		connect(popupMenu, SIGNAL(walletDeleted(QString)), this, SLOT(deleteWallet(QString)));
		connect(popupMenu, SIGNAL(walletChangePassword(QString)), this, SLOT(changeWalletPassword(QString)));
		connect(popupMenu, SIGNAL(walletCreated()), this, SLOT(createWallet()));
		popupMenu->exec(_iconView->mapToGlobal(pos));
		delete popupMenu;
	}
}


void KWalletManager::deleteWallet(const QString& walletName) {
	int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the wallet '%1'?", walletName),QString(),KStandardGuiItem::del());
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
		rc = KMessageBox::warningYesNo(this, i18n("Unable to close wallet cleanly. It is probably in use by other applications. Do you wish to force it closed?"), QString(), KGuiItem(i18n("Force Closure")), KGuiItem(i18n("Do Not Force")));
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
	KWallet::Wallet::changePassword(walletName, winId());
}


void KWalletManager::openWalletFile(const QString& path) {
	KWalletEditor *we = new KWalletEditor(path, true, this, "Wallet Editor");
	if (we->isOpen()) {
		connect(we, SIGNAL(editorClosed(KXmlGuiWindow*)),
			this, SLOT(editorClosed(KXmlGuiWindow*)));
		we->show();
	} else {
		KMessageBox::sorry(this, i18n("Error opening wallet %1.", path));
		delete we;
	}
}


void KWalletManager::openWallet() {
	QListWidgetItem *item = _iconView->currentItem();
	openWallet(item);
}

void KWalletManager::deleteWallet() {
	QListWidgetItem *item = _iconView->currentItem();
	if (item) {
		deleteWallet(item->text());
	}
}


void KWalletManager::openWallet(const QString& walletName) {
	openWallet(walletName, false);
}


void KWalletManager::openWallet(const QString& walletName, bool newWallet) {
	// Don't allow a wallet to open in two windows
	foreach (KXmlGuiWindow *w, _windows) {
		KWalletEditor *e = static_cast<KWalletEditor*>(w);
		if (e->isOpen() && e->_walletName == walletName) {
			w->raise();
			return;
		}
	}

	KWalletEditor *we = new KWalletEditor(walletName, false, this, "Wallet Editor");
	we->setNewWallet(newWallet);
	if (we->isOpen()) {
		connect(we, SIGNAL(editorClosed(KXmlGuiWindow*)),
			this, SLOT(editorClosed(KXmlGuiWindow*)));
		we->show();
		_windows.append(we);
	} else if (!newWallet) {
		KMessageBox::sorry(this, i18n("Error opening wallet %1.", walletName));
		delete we;
	}
}


void KWalletManager::openWallet(QListWidgetItem *item) {
	if (item) {
		openWallet(item->text());
	}
}


void KWalletManager::allWalletsClosed() {
	if (_tray) {
		_tray->setIconByName(QLatin1String( "wallet-closed" ));
		_tray->setToolTip( QLatin1String( "wallet-closed" ), i18n("KDE Wallet"), i18n("No wallets open."));
		_tray->setStatus(KStatusNotifierItem::Passive);
	}
	possiblyQuit();
}


void KWalletManager::possiblyQuit() {
	KConfig _cfg( QLatin1String(  "kwalletrc" ) );
	KConfigGroup cfg(&_cfg, "Wallet");
	if (_windows.isEmpty() &&
			!isVisible() &&
			!cfg.readEntry("Leave Manager Open", false) &&
			_kwalletdLaunch) {
		qApp->quit();
	}
}


void KWalletManager::editorClosed(KXmlGuiWindow* e) {
	_windows.removeAll(e);
}


void KWalletManager::possiblyRescan(const QString& app, const QString& oldOwner, const QString& newOwner) {
	Q_UNUSED( oldOwner );
	Q_UNUSED( newOwner );
	if (app == QLatin1String( "org.kde.kwalletd" )) {
		updateWalletDisplay();
	}
}

void KWalletManager::createWallet() {
	QString n;
	bool ok;
	QString txt = i18n("Please choose a name for the new wallet:");
	QRegExpValidator validator(QRegExp( QLatin1String( "^[\\w\\^\\&\\'\\@\\{\\}\\[\\]\\,\\$\\=\\!\\-\\#\\(\\)\\%\\.\\+\\_\\s]+$" )), this);

	if (!KWallet::Wallet::isEnabled()) {
		// FIXME: KMessageBox::warningYesNo(this, i1_8n("KWallet is not enabled.  Do you want to enable it?"), QString(), i18n("Enable"), i18n("Keep Disabled"));
		return;
	}

	do {
		n = KInputDialog::getText(i18n("New Wallet"), txt, QString(), &ok, this,
		                          &validator);

		if (!ok) {
			return;
		}

		if (!_iconView->findItems(n, Qt::MatchFixedString).isEmpty ()) {
			int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that wallet already exists. Try a new name?"), QString(), KGuiItem(i18n("Try New")), KGuiItem(i18n("Do Not Try")));
			if (rc == KMessageBox::No) {
				return;
			}
			n.clear();
		} else  {
			break;
		}
	} while (true);

	// Small race here - the wallet could be created on us already.
	if (!n.isEmpty()) {
		openWallet(n, true);
	}
}


void KWalletManager::shuttingDown() {
	_shuttingDown = true;
	qApp->quit();
}


void KWalletManager::setupWallet() {
	KToolInvocation::startServiceByDesktopName( QLatin1String( "kwalletconfig" ));
}


void KWalletManager::closeAllWallets() {
    m_kwalletdModule->closeAllWallets();
}

#include "kwalletmanager.moc"
