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

#ifndef KWALLETEDITOR_H
#define KWALLETEDITOR_H

#include "walletwidget.h"
#include <kwallet.h>
#include <kmainwindow.h>
#include <qstringlist.h>

class KAction;
class QIconViewItem;
class KHTMLPart;

class KWalletEditor : public KMainWindow {
	Q_OBJECT

	public:
		KWalletEditor(const QString& wallet, QWidget *parent = 0, const char* name = 0);
		virtual ~KWalletEditor();

		bool isOpen() const { return _w != 0L; }

	public slots:
		void walletClosed();
		void createFolder();
		void deleteFolder();

	private slots:
		void updateFolderList();
		void folderSelectionChanged(QIconViewItem *item);
		void updateEntries();
		void updateDetails();

	signals:
		void enableFolderActions(bool enable);
		void enableContextFolderActions(bool enable);
		void editorClosed(KMainWindow*);

	private:
		void createActions();
		QString _walletName;
		KWallet::Wallet *_w;
		WalletWidget *_ww;
		KAction *_newFolderAction, *_deleteFolderAction;
		KHTMLPart *_details;
		QStringList _entries;
};

#endif
