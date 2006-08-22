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
#include "walletconfigwidget.h"
#include <dcopclient.h>
#include <dcopref.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <kinputdialog.h>
#include <kpopupmenu.h>
#include <kwallet.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qspinbox.h>

typedef KGenericFactory<KWalletConfig, QWidget> KWalletFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_kwallet, KWalletFactory("kcmkwallet"))

KWalletConfig::KWalletConfig(QWidget *parent, const char *name, const QStringList&)
: KCModule(KWalletFactory::instance(), parent, name) {

	KAboutData *about =
		new KAboutData(I18N_NOOP("kcmkwallet"),
				I18N_NOOP("KDE Wallet Control Module"),
				0, 0, KAboutData::License_GPL,
				I18N_NOOP("(c) 2003 George Staikos"));
		about->addAuthor("George Staikos", 0, "staikos@kde.org");
	setAboutData( about );

	_cfg = new KConfig("kwalletrc", false, false);

	QVBoxLayout *vbox = new QVBoxLayout(this, 0, KDialog::spacingHint());
	vbox->add(_wcw = new WalletConfigWidget(this));

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
	connect(_wcw->_accessList, SIGNAL(contextMenuRequested(QListViewItem*, const QPoint&, int)), this, SLOT(contextMenuRequested(QListViewItem*, const QPoint&, int)));

	_wcw->_accessList->setAllColumnsShowFocus(true);
	updateWalletLists();
	load();

	if (DCOPClient::mainClient()->isApplicationRegistered("kwalletmanager")) {
		_wcw->_launch->hide();
	}

}


KWalletConfig::~KWalletConfig() {
	delete _cfg;
	_cfg = 0L;
}


void KWalletConfig::updateWalletLists() {
	QString p1, p2;
	p1 = _wcw->_localWallet->currentText();
	p2 = _wcw->_defaultWallet->currentText();

	_wcw->_localWallet->clear();
	_wcw->_defaultWallet->clear();

	QStringList wl = KWallet::Wallet::walletList();
	_wcw->_localWallet->insertStringList(wl);
	_wcw->_defaultWallet->insertStringList(wl);

	if (wl.contains(p1)) {
		_wcw->_localWallet->setCurrentText(p1);
	}

	if (wl.contains(p2)) {
		_wcw->_defaultWallet->setCurrentText(p2);
	}
}


QString KWalletConfig::newWallet() {
	bool ok;

	QString n = KInputDialog::getText(i18n("New Wallet"),
			i18n("Please choose a name for the new wallet:"),
			QString::null,
			&ok,
			this);

	if (!ok) {
		return QString::null;
	}

	KWallet::Wallet *w = KWallet::Wallet::openWallet(n);
	if (!w) {
		return QString::null;
	}

	delete w;
	return n;
}


void KWalletConfig::newLocalWallet() {
	QString n = newWallet();
	if (n.isEmpty()) {
		return;
	}

	updateWalletLists();

	_wcw->_localWallet->setCurrentText(n);

	emit changed(true);
}


void KWalletConfig::newNetworkWallet() {
	QString n = newWallet();
	if (n.isEmpty()) {
		return;
	}

	updateWalletLists();

	_wcw->_defaultWallet->setCurrentText(n);

	emit changed(true);
}


void KWalletConfig::launchManager() {
	if (!DCOPClient::mainClient()->isApplicationRegistered("kwalletmanager")) {
		KApplication::startServiceByDesktopName("kwalletmanager_show");
	} else {
		DCOPRef r("kwalletmanager", "kwalletmanager-mainwindow#1");
		r.send("show");
		r.send("raise");
	}
}


void KWalletConfig::configChanged() {
	emit changed(true);
}

void KWalletConfig::load() {
	load( false );
}

void KWalletConfig::load(bool useDefaults) {
	KConfigGroup config(_cfg, "Wallet");
	config.setReadDefaults( useDefaults );
	_wcw->_enabled->setChecked(config.readBoolEntry("Enabled", true));
	_wcw->_openPrompt->setChecked(config.readBoolEntry("Prompt on Open", true));
	_wcw->_launchManager->setChecked(config.readBoolEntry("Launch Manager", true));
	_wcw->_autocloseManager->setChecked(! config.readBoolEntry("Leave Manager Open", false));
	_wcw->_screensaverLock->setChecked(config.readBoolEntry("Close on Screensaver", false));
	_wcw->_autoclose->setChecked(!config.readBoolEntry("Leave Open", false));
	_wcw->_closeIdle->setChecked(config.readBoolEntry("Close When Idle", false));
	_wcw->_idleTime->setValue(config.readNumEntry("Idle Timeout", 10));
	if (config.hasKey("Default Wallet")) {
		_wcw->_defaultWallet->setCurrentText(config.readEntry("Default Wallet"));
	} else {
		_wcw->_defaultWallet->setCurrentItem(0);
	}
	if (config.hasKey("Local Wallet")) {
		_wcw->_localWalletSelected->setChecked( !config.readBoolEntry("Use One Wallet") );
		_wcw->_localWallet->setCurrentText(config.readEntry("Local Wallet"));
	} else {
		_wcw->_localWalletSelected->setChecked(false);
	}
	_wcw->_accessList->clear();
	_cfg->setGroup("Auto Deny");
	QStringList denykeys = _cfg->entryMap("Auto Deny").keys();
	_cfg->setGroup("Auto Allow");
	QStringList keys = _cfg->entryMap("Auto Allow").keys();
	for (QStringList::Iterator i = keys.begin(); i != keys.end(); ++i) {
		_cfg->setGroup("Auto Allow");
		QStringList apps = _cfg->readListEntry(*i);
		_cfg->setGroup("Auto Deny");
		QStringList denyapps = _cfg->readListEntry(*i);
		denykeys.remove(*i);
		QListViewItem *lvi = new QListViewItem(_wcw->_accessList, *i);
		for (QStringList::Iterator j = apps.begin(); j != apps.end(); ++j) {
			new QListViewItem(lvi, QString::null, *j, i18n("Always Allow"));
		}
		for (QStringList::Iterator j = denyapps.begin(); j != denyapps.end(); ++j) {
			new QListViewItem(lvi, QString::null, *j, i18n("Always Deny"));
		}
	}
	_cfg->setGroup("Auto Deny");
	for (QStringList::Iterator i = denykeys.begin(); i != denykeys.end(); ++i) {
		QStringList denyapps = _cfg->readListEntry(*i);
		QListViewItem *lvi = new QListViewItem(_wcw->_accessList, *i);
		for (QStringList::Iterator j = denyapps.begin(); j != denyapps.end(); ++j) {
			new QListViewItem(lvi, QString::null, *j, i18n("Always Deny"));
		}
	}
	emit changed(useDefaults);
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

	if (_wcw->_defaultWallet->currentItem() != -1) {
		config.writeEntry("Default Wallet", _wcw->_defaultWallet->currentText());
	} else {
		config.deleteEntry("Default Wallet");
	}

	// FIXME: won't survive a language change
	_cfg->deleteGroup("Auto Allow");
	_cfg->deleteGroup("Auto Deny");
	_cfg->setGroup("Auto Allow");
	for (QListViewItem *i = _wcw->_accessList->firstChild(); i; i = i->nextSibling()) {
		QStringList al;
		for (QListViewItem *j = i->firstChild(); j; j = j->nextSibling()) {
			if (j->text(2) == i18n("Always Allow")) {
				al << j->text(1);
			}
		}
		_cfg->writeEntry(i->text(0), al);
	}

	_cfg->setGroup("Auto Deny");
	for (QListViewItem *i = _wcw->_accessList->firstChild(); i; i = i->nextSibling()) {
		QStringList al;
		for (QListViewItem *j = i->firstChild(); j; j = j->nextSibling()) {
			if (j->text(2) == i18n("Always Deny")) {
				al << j->text(1);
			}
		}
		_cfg->writeEntry(i->text(0), al);
	}

	_cfg->sync();
	DCOPRef("kded", "kwalletd").call("reconfigure()");

	emit changed(false);
}


void KWalletConfig::defaults() {
	load( true );
}


QString KWalletConfig::quickHelp() const {
	return i18n("This configuration module allows you to configure the KDE wallet system.");
}


void KWalletConfig::contextMenuRequested(QListViewItem *item, const QPoint& pos, int col) {
	Q_UNUSED(col)
	if (item && item->parent()) {
		KPopupMenu *m = new KPopupMenu(this);
		m->insertTitle(item->parent()->text(0));
		m->insertItem(i18n("&Delete"), this, SLOT(deleteEntry()), Key_Delete);
		m->popup(pos);
	}
}


void KWalletConfig::deleteEntry() {
	QListViewItem *item = _wcw->_accessList->selectedItem();
	if (item) {
		delete item;
		emit changed(true);
	}
}

#include "konfigurator.moc"

