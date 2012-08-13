/*
   Copyright (C) 2003-2005 George Staikos <staikos@kde.org>
   Copyright (C) 2005 Isaac Clerencia <isaac@warp.es>

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

#ifndef KWALLETEDITOR_H
#define KWALLETEDITOR_H

#include "ui_walletwidget.h"
#include <kwallet.h>
#include <kxmlguiwindow.h>
#include <QStringList>
#include <QLabel>

class QTreeWidgetItem;
class QCheckBox;
class KWalletEntryList;
class KWMapEditor;
class KAction;
class WalletWidget : public QWidget, public Ui::WalletWidget
{
public:
  WalletWidget( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};


class KWalletEditor : public KXmlGuiWindow {
	Q_OBJECT

	public:
		KWalletEditor(const QString& wallet, bool isPath, QWidget *parent = 0, const char* name = 0);
		virtual ~KWalletEditor();

		bool isOpen() const { return _w != 0L; }

		void setNewWallet(bool newWallet);

	public slots:
		void walletClosed();
		void createFolder();
		void deleteFolder();

	private slots:
		void updateFolderList(bool checkEntries = false);
		void entrySelectionChanged(QTreeWidgetItem *item);
		void listItemChanged(QTreeWidgetItem *, int column);
		void listContextMenuRequested(const QPoint& pos);
		void updateEntries(const QString& folder);

		void newEntry();
		void renameEntry();
		void deleteEntry();
		void entryEditted();
		void restoreEntry();
		void saveEntry();

		void changePassword();

		void walletOpened(bool success);
		void hidePasswordContents();
		void showPasswordContents();
		void showHideMapEditorValue(bool show);

		void saveAs();
		void exportXML();
		void importXML();
		void importWallet();

		void copyPassword();

	signals:
		void enableWalletActions(bool enable);
		void enableFolderActions(bool enable);
		void enableContextFolderActions(bool enable);
		void editorClosed(KXmlGuiWindow*);

	public:
		QString _walletName;

	private:
		void createActions();
		bool _nonLocal;
		KWallet::Wallet *_w;
		WalletWidget *_ww;
		KWalletEntryList *_entryList;
		bool _walletIsOpen;
		QAction *_newFolderAction, *_deleteFolderAction;
		QAction *_passwordAction, *_exportAction, *_saveAsAction, *_mergeAction, *_importAction;
		KAction *_newEntryAction, *_renameEntryAction, *_deleteEntryAction;
		QAction *_copyPassAction;
		QLabel*_details;
		QString _currentFolder;
		QMap<QString,QString> _currentMap; // save memory by storing
						   // only the most recent map.
		KWMapEditor *_mapEditor;
		QCheckBox *_mapEditorShowHide;
		bool _newWallet;
		KMenu *_contextMenu;
        QTreeWidgetItem *_displayedItem; // used to find old item when selection just changed
};

#endif
