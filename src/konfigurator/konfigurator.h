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

#ifndef _KWALLETKONFIGURATOR_H
#define _KWALLETKONFIGURATOR_H
#include <kcmodule.h>
#include <ksharedconfig.h>
#include "ui_walletconfigwidget.h"

class WalletConfigWidget : public QWidget, public Ui::WalletConfigWidget
{
public:
  WalletConfigWidget( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};


class KWalletConfig : public KCModule {
	Q_OBJECT
	public:
		explicit KWalletConfig(QWidget *parent = 0L, const QVariantList& = QVariantList());
		virtual ~KWalletConfig();

		void load();
		void save();
		void defaults();

		QString quickHelp() const;

	public slots:
		void configChanged();
		void launchManager();
		void newLocalWallet();
		void newNetworkWallet();
		void updateWalletLists();
		QString newWallet();
		void deleteEntry();
		void customContextMenuRequested(const QPoint& pos);

	private:
		WalletConfigWidget *_wcw;
		KSharedConfig::Ptr _cfg;
};

#endif
