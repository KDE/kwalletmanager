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
#include "kwalletmanagerwidget.h"
#include "kwalletpopup.h"
#include "kwalleteditor.h"
#include "allyourbase.h"
#include "kwallet_interface.h"
#include "registercreateactionmethod.h"

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
    RegisterCreateActionsMethod::createActions(actionCollection());

    setObjectName(QLatin1String( name ) );
    QDBusConnection::sessionBus().registerObject(QLatin1String( "/KWalletManager" ), this, QDBusConnection::ExportScriptableSlots);
	KGlobal::dirs()->addResourceType("kwallet", 0, QLatin1String( "share/apps/kwallet" ));
	_kwalletdLaunch = false;
	_shuttingDown = false;
	m_kwalletdModule = 0;
	KConfig cfg( QLatin1String( "kwalletrc" )); // not sure why this setting isn't in kwalletmanagerrc...
	KConfigGroup walletConfigGroup(&cfg, "Wallet");
	if (walletConfigGroup.readEntry("Launch Manager", false)) {
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

    _managerWidget = new KWalletManagerWidget(this);

	updateWalletDisplay();
	setCentralWidget(_managerWidget);
	setAutoSaveSettings(QLatin1String("MainWindow"), true);
//  	_managerWidget->setMinimumSize(320, 200);

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
        connect( m_kwalletdModule, SIGNAL(walletCreated(QString)), this, SLOT(walletCreated(QString)));
	// FIXME: slight race - a wallet can open, then we get launched, but the
	//        wallet closes before we are done opening.  We will then stay
	//        open.  Must check that a wallet is still open here.

	QAction *action = actionCollection()->addAction(QLatin1String( "wallet_create" ));
	action->setText(i18n("&New Wallet..."));
	action->setIcon(KIcon( QLatin1String( "kwalletmanager" )));
	connect(action, SIGNAL(triggered()), SLOT(createWallet()));

    action = actionCollection()->addAction(QLatin1String( "wallet_open") );
    action->setText(i18n("Open Wallet..."));
    connect(action, SIGNAL(triggered()), this, SLOT(openWallet()));
    
    action = actionCollection()->addAction(QLatin1String( "wallet_delete" ));
    action->setText(i18n("&Delete Wallet..."));
    action->setIcon(KIcon( QLatin1String( "trash-empty" )));
    connect(action, SIGNAL(triggered()), SLOT(deleteWallet()));
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

    setupGUI( Keys | Save | Create, QLatin1String( "kwalletmanager.rc" ));
    setStandardToolBarMenuEnabled(false);

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
    createGUI( QLatin1String( "kwalletmanager.rc" ));
}


void KWalletManager::updateWalletDisplay() {
    _managerWidget->updateWalletDisplay();
}

void KWalletManager::walletCreated(const QString& newWalletName)
{
    _managerWidget->updateWalletDisplay(newWalletName);
}

void KWalletManager::contextMenu(const QPoint& ) {
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

void KWalletManager::changeWalletPassword(const QString &walletName)
{
    KWallet::Wallet::changePassword(walletName, effectiveWinId());
}


void KWalletManager::openWalletFile(const QString& path) {
    if (!_managerWidget->openWalletFile(path)) {
		KMessageBox::sorry(this, i18n("Error opening wallet %1.", path));
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

		if (_managerWidget->hasWallet(n)) {
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
        // attempt open the wallet to create it, then dispose it
        // as it'll appear in on the main window via the walletCreated signal
        // emmitted by the kwalletd
        KWallet::Wallet::openWallet(n, effectiveWinId());
    }
}

void KWalletManager::deleteWallet()
{
    QString walletName = _managerWidget->activeWalletName();
    int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the wallet '%1'?", walletName),QString(),KStandardGuiItem::del());
    if (rc != KMessageBox::Continue) {
        return;
    }
    rc = KWallet::Wallet::deleteWallet(walletName);
    if (rc != 0) {
        KMessageBox::sorry(this, i18n("Unable to delete the wallet. Error code was %1.", rc));
    }
}

void KWalletManager::openWallet(const QString& walletName)
{
    _managerWidget->openWallet(walletName);
}

void KWalletManager::openWallet()
{
    qWarning("TODO: implement openWallet from file");
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
