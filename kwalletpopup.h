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

#ifndef KWALLETPOPUP_H
#define KWALLETPOPUP_H

#include <kpopupmenu.h>
#include <qmap.h>
#include <qstring.h>

class KWalletPopup : public KPopupMenu {
	Q_OBJECT

	public:
		KWalletPopup(const QString& wallet, QWidget *parent = 0, const char* name = 0);
		virtual ~KWalletPopup();

	public slots:
		void openWallet();
		void deleteWallet();
		void closeWallet();
		void createWallet();
		void changeWalletPassword();
		void disconnectApp(int);

	signals:
		void walletOpened(const QString& walletName);
		void walletClosed(const QString& walletName);
		void walletDeleted(const QString& walletName);
		void walletCreated();
		void walletChangePassword(const QString& walletName);

	private:
		QString _walletName;
		QMap<int,QString> _appMap;
};

#endif
