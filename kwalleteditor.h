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
class QListViewItem;
class KHTMLPart;
class KWalletFolderIconView;
class KWalletEntryList;
class KWMapEditor;

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
		void entrySelectionChanged(QListViewItem *item);
		void listItemRenamed(QListViewItem *, int, const QString&);
		void listContextMenuRequested(QListViewItem *item, const QPoint& pos, int col);
		void iconContextMenuRequested(QIconViewItem *item, const QPoint& pos);
		void updateEntries();
		void updateEntries(const QString& folder);
		void updateDetails();

		void newEntry();
		void renameEntry();
		void deleteEntry();
		void entryEditted();
		void restoreEntry();
		void saveEntry();

		void changePassword();
		void slotConfigureKeys();

		void walletOpened(bool success);
		void hidePasswordContents();
		void showPasswordContents();

	signals:
		void enableFolderActions(bool enable);
		void enableContextFolderActions(bool enable);
		void editorClosed(KMainWindow*);

	private:
		void createActions();
		QString _walletName;
		KWallet::Wallet *_w;
		WalletWidget *_ww;
		KWalletFolderIconView *_folderView;
		KWalletEntryList *_entryList;
		KAction *_newFolderAction, *_deleteFolderAction;
		KAction *_passwordAction;
		KHTMLPart *_details;
		QStringList _entries;
		QListViewItem *_passItems, *_mapItems, *_binaryItems, *_unknownItems;
		QMap<QString,QString> _currentMap; // save memory by storing
						   // only the most recent map.
		KWMapEditor *_mapEditor;
};

#endif
