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

#ifndef KWALLETMANAGER_H
#define KWALLETMANAGER_H

#include <kmainwindow.h>
#include <dcopobject.h>
#include <qptrlist.h>

class KSystemTray;
class KIconView;
class QIconViewItem;
class DCOPRef;


class KWalletManager : public KMainWindow, public DCOPObject {
	Q_OBJECT
	K_DCOP

	public:
		KWalletManager(QWidget *parent = 0, const char* name = 0, WFlags f = 0);
		virtual ~KWalletManager();

	k_dcop:
		ASYNC updateWalletDisplay();

	public slots:
		void deleteWallet(const QString& walletName);
		void closeWallet(const QString& walletName);
		void openWallet(const QString& walletName);
		void openWallet(QIconViewItem *item);
		void contextMenu(QIconViewItem *item, const QPoint& pos);

	private:
	k_dcop:
		ASYNC allWalletsClosed();

	private slots:
		void possiblyQuit();
		void editorClosed(KMainWindow* e);

	private:
		KSystemTray *_tray;
		KIconView *_iconView;
		DCOPRef *_dcopRef;
		QPtrList<KMainWindow> _windows;
};

#endif
