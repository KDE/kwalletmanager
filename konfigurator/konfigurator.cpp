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

#include "konfigurator.h"
#include "walletconfigwidget.h"
#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qspinbox.h>

typedef KGenericFactory<KWalletConfig, QWidget> KWalletFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_kwallet, KWalletFactory("kcmkwallet"));

KWalletConfig::KWalletConfig(QWidget *parent, const char *name, const QStringList&)
: KCModule(KWalletFactory::instance(), parent, name) {

	_cfg = new KConfig("kwalletrc", false, false);

	QVBoxLayout *vbox = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
	vbox->add(_wcw = new WalletConfigWidget(this));
	connect(_wcw->_launchManager, SIGNAL(clicked()), this, SLOT(configChanged()));
	connect(_wcw->_leaveManagerOpen, SIGNAL(clicked()), this, SLOT(configChanged()));
	connect(_wcw->_leaveOpen, SIGNAL(clicked()), this, SLOT(configChanged()));
	connect(_wcw->_closeIdle, SIGNAL(clicked()), this, SLOT(configChanged()));
	connect(_wcw->_idleTime, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
	load();
}


KWalletConfig::~KWalletConfig() {
	delete _cfg;
	_cfg = 0L;
}


void KWalletConfig::configChanged() {
    emit changed(true);
}


void KWalletConfig::load() {
	KConfigGroup config(_cfg, "Wallet");
	_wcw->_launchManager->setChecked(config.readBoolEntry("Launch Manager", true));
	_wcw->_leaveManagerOpen->setChecked(config.readBoolEntry("Leave Manager Open", false));
	_wcw->_leaveOpen->setChecked(config.readBoolEntry("Leave Open", false));
	_wcw->_closeIdle->setChecked(config.readBoolEntry("Close When Idle", false));
	_wcw->_idleTime->setValue(config.readNumEntry("Idle Timeout", 10));
	emit changed(false);
}


void KWalletConfig::save() {
	KConfigGroup config(_cfg, "Wallet");
	config.writeEntry("Launch Manager", _wcw->_launchManager->isChecked());
	config.writeEntry("Leave Manager Open", _wcw->_leaveManagerOpen->isChecked());
	config.writeEntry("Leave Open", _wcw->_leaveOpen->isChecked());
	config.writeEntry("Close When Idle", _wcw->_closeIdle->isChecked());
	config.writeEntry("Idle Timeout", _wcw->_idleTime->value());
	emit changed(false);
}


void KWalletConfig::defaults() {
	_wcw->_launchManager->setChecked(true);
	_wcw->_leaveManagerOpen->setChecked(false);
	_wcw->_leaveOpen->setChecked(false);
	_wcw->_closeIdle->setChecked(false);
	_wcw->_idleTime->setValue(10);
	emit changed(true);
}


QString KWalletConfig::quickHelp() const {
	return "FIXME";
}


const KAboutData *KWalletConfig::aboutData() const {
KAboutData *about =
	new KAboutData(I18N_NOOP("kcmkwallet"),
			I18N_NOOP("KDE Wallet Control Module"),
			0, 0, KAboutData::License_GPL,
			I18N_NOOP("(c) 2003 George Staikos"));
	about->addAuthor("George Staikos", 0, "staikos@kde.org");
return about;
}


#include "konfigurator.moc"

