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
#include "konfigurator.h"

#include <QtDBus/QtDBus>
#include <kaboutdata.h>
#include <kapplication.h>
#include <ksharedconfig.h>
#include <kdialog.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kinputdialog.h>
#include <kmenu.h>
#include <kwallet.h>

#include <QCheckBox>
#include <q3listview.h>
#include <QPushButton>
//Added by qt3to4:
#include <QVBoxLayout>
#include <ktoolinvocation.h>
#include <kconfiggroup.h>
#define KWALLETMANAGERINTERFACE "org.kde.KWallet"

K_PLUGIN_FACTORY(KWalletFactory, registerPlugin<KWalletConfig>();)
K_EXPORT_PLUGIN(KWalletFactory("kcmkwallet"))


KWalletConfig::KWalletConfig(QWidget *parent, const QVariantList& args)
: KCModule(KWalletFactory::componentData(), parent, args),
  _cfg(KSharedConfig::openConfig("kwalletrc", KConfig::NoGlobals)) {

	KAboutData *about =
		new KAboutData(I18N_NOOP("kcmkwallet"), 0,
				ki18n("KDE Wallet Control Module"),
				0, KLocalizedString(), KAboutData::License_GPL,
				ki18n("(c) 2003 George Staikos"));
		about->addAuthor(ki18n("George Staikos"), KLocalizedString(), "staikos@kde.org");
	setAboutData( about );

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setSpacing(KDialog::spacingHint());
	vbox->setMargin(0);
        _wcw = new WalletConfigWidget(this);
	vbox->addWidget(_wcw);

	connect(_wcw->_enabled, SIGNAL(clicked()), this, SLOT(configChanged()));
	connect(_wcw->_launchManager, SIGNAL(clicked()), this, SLOT(configChanged()));
	connect(_wcw->_autocloseManager, SIGNAL(clicked()), this, SLOT(configChanged()));
	connect(_wcw->_autoclose, SIGNAL(clicked()), this, SLOT(configChanged()));
	connect(_wcw->_closeIdle, SIGNAL(clicked()), this, SLOT(configChanged()));
	connect(_wcw->_openPrompt, SIGNAL(clicked()), this, SLOT(configChanged()));
	connect(_wcw->_screensaverLock, SIGNAL(clicked()), this, SLOT(configChanged()));
	connect(_wcw->_localWalletSelected, SIGNAL(clicked()), this, SLOT(configChanged()));
	connect(_wcw->_idleTime, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
	connect(_wcw->_launch, SIGNAL(clicked()), this, SLOT(launchManager()));
	connect(_wcw->_newWallet, SIGNAL(clicked()), this, SLOT(newNetworkWallet()));
	connect(_wcw->_newLocalWallet, SIGNAL(clicked()), this, SLOT(newLocalWallet()));
	connect(_wcw->_localWallet, SIGNAL(activated(int)), this, SLOT(configChanged()));
	connect(_wcw->_defaultWallet, SIGNAL(activated(int)), this, SLOT(configChanged()));
	connect(_wcw->_accessList, SIGNAL(contextMenuRequested(Q3ListViewItem*, const QPoint&, int)), this, SLOT(contextMenuRequested(Q3ListViewItem*, const QPoint&, int)));

	_wcw->_accessList->setAllColumnsShowFocus(true);
	updateWalletLists();

	if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kwalletmanager")) {
		_wcw->_launch->hide();
	}

}


KWalletConfig::~KWalletConfig() {
}


void KWalletConfig::updateWalletLists() {
        const QString p1( _wcw->_localWallet->currentText() );
	const QString p2( _wcw->_defaultWallet->currentText() );

	_wcw->_localWallet->clear();
	_wcw->_defaultWallet->clear();

	const QStringList wl = KWallet::Wallet::walletList();
	_wcw->_localWallet->addItems(wl);
	_wcw->_defaultWallet->addItems(wl);

	if (wl.contains(p1)) {
		_wcw->_localWallet->setCurrentText(p1);
	}

	if (wl.contains(p2)) {
		_wcw->_defaultWallet->setCurrentText(p2);
	}
}


QString KWalletConfig::newWallet() {
	bool ok;

	const QString n = KInputDialog::getText(i18n("New Wallet"),
			i18n("Please choose a name for the new wallet:"),
			QString(),
			&ok,
			this);

	if (!ok) {
		return QString();
	}

	KWallet::Wallet *w = KWallet::Wallet::openWallet(n, topLevelWidget()->winId());
	if (!w) {
		return QString();
	}

	delete w;
	return n;
}


void KWalletConfig::newLocalWallet() {
	const QString n = newWallet();
	if (n.isEmpty()) {
		return;
	}

	updateWalletLists();

	_wcw->_localWallet->setCurrentText(n);

	emit changed(true);
}


void KWalletConfig::newNetworkWallet() {
	const QString n = newWallet();
	if (n.isEmpty()) {
		return;
	}

	updateWalletLists();

	_wcw->_defaultWallet->setCurrentText(n);

	emit changed(true);
}


void KWalletConfig::launchManager() {
	if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kwalletmanager")) {
		KToolInvocation::startServiceByDesktopName("kwalletmanager_show");
	} else {
             QDBusInterface kwalletd("org.kde.kwalletmanager", "/kwalletmanager/MainWindow_1");
             kwalletd.call( "show");
             kwalletd.call( "raise" );
	}
}


void KWalletConfig::configChanged() {
	emit changed(true);
}

void KWalletConfig::load() {
	KConfigGroup config(_cfg, "Wallet");
	_wcw->_enabled->setChecked(config.readEntry("Enabled", true));
	_wcw->_openPrompt->setChecked(config.readEntry("Prompt on Open", true));
	_wcw->_launchManager->setChecked(config.readEntry("Launch Manager", true));
	_wcw->_autocloseManager->setChecked(! config.readEntry("Leave Manager Open", false));
	_wcw->_screensaverLock->setChecked(config.readEntry("Close on Screensaver", false));
	_wcw->_autoclose->setChecked(!config.readEntry("Leave Open", false));
	_wcw->_closeIdle->setChecked(config.readEntry("Close When Idle", false));
	_wcw->_idleTime->setValue(config.readEntry("Idle Timeout", 10));
	if (config.hasKey("Default Wallet")) {
		_wcw->_defaultWallet->setCurrentText(config.readEntry("Default Wallet"));
	} else {
		_wcw->_defaultWallet->setCurrentIndex(0);
	}
	if (config.hasKey("Local Wallet")) {
		_wcw->_localWalletSelected->setChecked( !config.readEntry("Use One Wallet", false) );
		_wcw->_localWallet->setCurrentText(config.readEntry("Local Wallet"));
	} else {
		_wcw->_localWalletSelected->setChecked(false);
	}
	_wcw->_accessList->clear();
	KConfigGroup ad(_cfg, "Auto Deny");
	KConfigGroup aa(_cfg, "Auto Allow");
	QStringList denykeys = ad.entryMap().keys();
	const QStringList keys = aa.entryMap().keys();
	for (QStringList::const_iterator i = keys.begin(); i != keys.end(); ++i) {
		const QStringList apps = aa.readEntry(*i,QStringList());
		const QStringList denyapps = ad.readEntry(*i, QStringList());
		denykeys.removeAll(*i);
		Q3ListViewItem *lvi = new Q3ListViewItem(_wcw->_accessList, *i);
		for (QStringList::const_iterator j = apps.begin(); j != apps.end(); ++j) {
			new Q3ListViewItem(lvi, QString(), *j, i18n("Always Allow"));
		}
		for (QStringList::const_iterator j = denyapps.begin(); j != denyapps.end(); ++j) {
			new Q3ListViewItem(lvi, QString(), *j, i18n("Always Deny"));
		}
	}
	for (QStringList::const_iterator i = denykeys.constBegin(); i != denykeys.constEnd(); ++i) {
		const QStringList denyapps = ad.readEntry(*i,QStringList());
		Q3ListViewItem *lvi = new Q3ListViewItem(_wcw->_accessList, *i);
		for (QStringList::const_iterator j = denyapps.begin(); j != denyapps.end(); ++j) {
			new Q3ListViewItem(lvi, QString(), *j, i18n("Always Deny"));
		}
	}
	emit changed(false);
}


void KWalletConfig::save() {
	KConfigGroup config(_cfg, "Wallet");
	config.writeEntry("Enabled", _wcw->_enabled->isChecked());
	config.writeEntry("Launch Manager", _wcw->_launchManager->isChecked());
	config.writeEntry("Leave Manager Open", !_wcw->_autocloseManager->isChecked());
	config.writeEntry("Leave Open", !_wcw->_autoclose->isChecked());
	config.writeEntry("Close When Idle", _wcw->_closeIdle->isChecked());
	config.writeEntry("Idle Timeout", _wcw->_idleTime->value());
	config.writeEntry("Prompt on Open", _wcw->_openPrompt->isChecked());
	config.writeEntry("Close on Screensaver", _wcw->_screensaverLock->isChecked());

	config.writeEntry("Use One Wallet", !_wcw->_localWalletSelected->isChecked());
	if (_wcw->_localWalletSelected->isChecked()) {
		config.writeEntry("Local Wallet", _wcw->_localWallet->currentText());
	} else {
		config.deleteEntry("Local Wallet");
	}

	if (_wcw->_defaultWallet->currentIndex() != -1) {
		config.writeEntry("Default Wallet", _wcw->_defaultWallet->currentText());
	} else {
		config.deleteEntry("Default Wallet");
	}

	// FIXME: won't survive a language change
	_cfg->deleteGroup("Auto Allow");
	_cfg->deleteGroup("Auto Deny");
	config  = _cfg->group("Auto Allow");
	for (Q3ListViewItem *i = _wcw->_accessList->firstChild(); i; i = i->nextSibling()) {
		QStringList al;
		for (Q3ListViewItem *j = i->firstChild(); j; j = j->nextSibling()) {
			if (j->text(2) == i18n("Always Allow")) {
				al << j->text(1);
			}
		}
		config.writeEntry(i->text(0), al);
	}

	config = _cfg->group("Auto Deny");
	for (Q3ListViewItem *i = _wcw->_accessList->firstChild(); i; i = i->nextSibling()) {
		QStringList al;
		for (Q3ListViewItem *j = i->firstChild(); j; j = j->nextSibling()) {
			if (j->text(2) == i18n("Always Deny")) {
				al << j->text(1);
			}
		}
		config.writeEntry(i->text(0), al);
	}

	_cfg->sync();

        // this restarts kwalletd if necessary
	if (KWallet::Wallet::isEnabled()) {
            QDBusInterface kwalletd("org.kde.kwalletd", "/modules/kwalletd", KWALLETMANAGERINTERFACE);
            kwalletd.call( "reconfigure" );
        }
	emit changed(false);
}


void KWalletConfig::defaults() {
	_wcw->_enabled->setChecked(true);
	_wcw->_openPrompt->setChecked(true);
	_wcw->_launchManager->setChecked(true);
	_wcw->_autocloseManager->setChecked(false);
	_wcw->_screensaverLock->setChecked(false);
	_wcw->_autoclose->setChecked(true);
	_wcw->_closeIdle->setChecked(false);
	_wcw->_idleTime->setValue(10);
	_wcw->_defaultWallet->setCurrentIndex(0);
	_wcw->_localWalletSelected->setChecked(false);
        _wcw->_localWallet->setCurrentIndex( 0 );
	_wcw->_accessList->clear();
	emit changed(true);
}


QString KWalletConfig::quickHelp() const {
	return i18n("This configuration module allows you to configure the KDE wallet system.");
}


void KWalletConfig::contextMenuRequested(Q3ListViewItem *item, const QPoint& pos, int col) {
	Q_UNUSED(col)
	if (item && item->parent()) {
		KMenu *m = new KMenu(this);
		m->addTitle(item->parent()->text(0));
		m->addAction(i18n("&Delete"), this, SLOT(deleteEntry()), Qt::Key_Delete);
		m->exec(pos);
                delete m;
	}
}


void KWalletConfig::deleteEntry() {
	Q3ListViewItem *item = _wcw->_accessList->selectedItem();
	if (item) {
		delete item;
		emit changed(true);
	}
}

#include "konfigurator.moc"

