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


#include "kwalleteditor.h"

#include <dcopref.h>
#include <dcopclient.h>
#include <kaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconview.h>
#include <kinputdialog.h>
#include <kstdaction.h>
#include <kmessagebox.h>
#include <khtml_part.h>

#include <qwidgetstack.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qptrstack.h>


KWalletEditor::KWalletEditor(const QString& wallet, QWidget *parent, const char *name)
: KMainWindow(parent, name), _walletName(wallet) {
	_ww = new WalletWidget(this, "Wallet Widget");
	setCentralWidget(_ww);

	connect(_ww->_folderView, SIGNAL(selectionChanged(QIconViewItem*)),
		this, SLOT(folderSelectionChanged(QIconViewItem*)));

	_w = KWallet::Wallet::openWallet(wallet);
	if (_w) {
		connect(_w, SIGNAL(walletClosed()), this, SLOT(walletClosed()));
		updateFolderList();
	}

	QVBoxLayout *box = new QVBoxLayout(_ww->_folderDetails);
	_details = new KHTMLPart(_ww->_folderDetails, "Folder Details");
	box->addWidget(_details->widget());
	_details->setJScriptEnabled(false);
	_details->setJavaEnabled(false);
	_details->setMetaRefreshEnabled(false);
	_details->setPluginsEnabled(false);
	_details->setOnlyLocalReferences(true);

	_ww->_entryList->setEnabled(false);
	_ww->_folderDetails->setEnabled(false);
	_ww->_entryStack->setEnabled(false);

	createActions();
	createGUI("kwalleteditor.rc");
}


KWalletEditor::~KWalletEditor() {
	emit editorClosed(this);
	delete _newFolderAction;
	_newFolderAction = 0L;
	delete _deleteFolderAction;
	_deleteFolderAction = 0L;
	delete _w;
	_w = 0L;
}


void KWalletEditor::createActions() {
	_newFolderAction = new KAction(i18n("New &Folder..."), 0, 0, this,
			SLOT(createFolder()), actionCollection(),
			"create_folder");
	connect(this, SIGNAL(enableFolderActions(bool)),
		_newFolderAction, SLOT(setEnabled(bool)));

	_deleteFolderAction = new KAction(i18n("Delete &Folder"), 0, Key_Delete,
			this, SLOT(deleteFolder()), actionCollection(),
			"delete_folder");
	connect(this, SIGNAL(enableContextFolderActions(bool)),
		_deleteFolderAction, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(enableFolderActions(bool)),
		_deleteFolderAction, SLOT(setEnabled(bool)));

	KStdAction::close(this, SLOT(close()), actionCollection());

	emit enableFolderActions(_w != 0L);
	emit enableContextFolderActions(false);
}


void KWalletEditor::walletClosed() {
	delete _w;
	_w = 0L;
	_ww->_folderView->clear();
	_ww->setEnabled(false);
	emit enableFolderActions(false);
	// FIXME
}


void KWalletEditor::updateFolderList() {
QStringList fl = _w->folderList();
QPtrStack<QIconViewItem> trash;

	for (QIconViewItem *item = _ww->_folderView->firstItem(); item; item = item->nextItem()) {
		if (!fl.contains(item->text())) {
			trash.push(item);
		}
	}

	trash.setAutoDelete(true);
	trash.clear();

	for (QStringList::Iterator i = fl.begin(); i != fl.end(); ++i) {
		if (!_ww->_folderView->findItem(*i)) {
			new QIconViewItem(_ww->_folderView, *i);
		}
	}
}


void KWalletEditor::deleteFolder() {
	// FIXME: prompt for confirmation!
	if (_w) {
		QIconViewItem *ivi = _ww->_folderView->currentItem();
		if (ivi) {
			// FIXME: error handling
			_w->removeFolder(ivi->text());
			updateFolderList();
		}
	}
}


void KWalletEditor::createFolder() {
	if (_w) {
		QString n;

		do {
			n = KInputDialog::getText(i18n("New Folder..."),
					i18n("Please choose a name for the new folder..."));

			if (n.isEmpty()) {
				return;
			}

			if (_ww->_folderView->findItem(n)) {
				int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that folder name is in use.  Try again?"));
				if (rc == KMessageBox::Yes) {
					continue;
				}
			}
		} while (false);

		_w->createFolder(n);
		updateFolderList();
	}
}


void KWalletEditor::folderSelectionChanged(QIconViewItem *item) {
	emit enableContextFolderActions(item != 0L);

	if (item && _w) {
		_w->setFolder(item->text());
		_entries = _w->entryList();
		updateDetails();
		updateEntries();
	} else {
		_details->begin();
		_details->end();
		_ww->_entryList->clear();
	}
	_ww->_entryList->setEnabled(item && _w);
	_ww->_folderDetails->setEnabled(item && _w);
	_ww->_entryStack->setEnabled(item && _w);
}


void KWalletEditor::updateDetails() {
	static const QString page = "<html><body>"
			"<span style=\"text-align: center\">Folder: <b>%1</b>"
			"</span><br/>"
			"<ul>"
			"<li>Contains %2 items.</li>"
			"</ul>"
			"<br/>FIXME</body></html>";

	_details->begin();
	_details->write(page
			.arg(_ww->_folderView->currentItem()->text())
			.arg(_entries.count()));
	_details->end();
}


void KWalletEditor::updateEntries() {
	_ww->_entryList->clear();
	QListViewItem *passItems, *mapItems, *binaryItems, *unknownItems;
	passItems = new QListViewItem(_ww->_entryList, i18n("Passwords"));
	mapItems = new QListViewItem(_ww->_entryList, i18n("Maps"));
	binaryItems = new QListViewItem(_ww->_entryList, i18n("Binary Data"));
	unknownItems = new QListViewItem(_ww->_entryList, i18n("Unknown"));
	
	for (QStringList::Iterator i = _entries.begin(); i != _entries.end(); ++i) {
		switch (_w->entryType(*i)) {
		case KWallet::Wallet::Password:
			new QListViewItem(passItems, *i);
			break;
		case KWallet::Wallet::Stream:
			new QListViewItem(mapItems, *i);
			break;
		case KWallet::Wallet::Map:
			new QListViewItem(binaryItems, *i);
			break;
		case KWallet::Wallet::Unknown:
		default:
			new QListViewItem(unknownItems, *i);
			break;
		}
	}
}


#include "kwalleteditor.moc"

