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
#include "allyourbase.h"

#include <dcopclient.h>
#include <dcopref.h>
#include <kaction.h>
#include <kapplication.h>
#include <kiconview.h>
#include <klineeditdlg.h>
#include <khtml_part.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kstdaction.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qptrstack.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qwidgetstack.h>

#include <assert.h>

KWalletEditor::KWalletEditor(const QString& wallet, QWidget *parent, const char *name)
: KMainWindow(parent, name), _walletName(wallet) {
	_ww = new WalletWidget(this, "Wallet Widget");
	setCentralWidget(_ww);

	connect(_ww->_folderView, SIGNAL(selectionChanged(QIconViewItem*)),
		this, SLOT(folderSelectionChanged(QIconViewItem*)));
	connect(_ww->_entryList, SIGNAL(selectionChanged(QListViewItem*)),
		this, SLOT(entrySelectionChanged(QListViewItem*)));
	connect(_ww->_entryList,
		SIGNAL(contextMenuRequested(QListViewItem*,const QPoint&,int)),
		this,
		SLOT(listContextMenuRequested(QListViewItem*,const QPoint&,int)));
	connect(_ww->_entryList,
		SIGNAL(itemRenamed(QListViewItem*, int, const QString&)),
		this,
		SLOT(listItemRenamed(QListViewItem*, int, const QString&)));
	connect(_ww->_folderView,
		SIGNAL(contextMenuRequested(QIconViewItem*,const QPoint&)),
		this,
		SLOT(iconContextMenuRequested(QIconViewItem*,const QPoint&)));

	connect(_ww->_passwordValue, SIGNAL(textChanged()),
		this, SLOT(entryEditted()));

	connect(_ww->_undoChanges, SIGNAL(clicked()),
		this, SLOT(restoreEntry()));
	connect(_ww->_saveChanges, SIGNAL(clicked()),
		this, SLOT(saveEntry()));

	_w = KWallet::Wallet::openWallet(wallet);
	if (_w) {
		connect(_w, SIGNAL(walletClosed()), this, SLOT(walletClosed()));
		connect(_w, SIGNAL(folderUpdated(const QString&)), this, SLOT(updateEntries(const QString&)));
		connect(_w, SIGNAL(folderListUpdated()), this, SLOT(updateFolderList()));
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

	setCaption(wallet);
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
	_newFolderAction = new KAction(i18n("&New Folder..."), 0, 0, this,
			SLOT(createFolder()), actionCollection(),
			"create_folder");
	connect(this, SIGNAL(enableFolderActions(bool)),
		_newFolderAction, SLOT(setEnabled(bool)));

	_deleteFolderAction = new KAction(i18n("&Delete Folder"), 0, Key_Delete,
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
	folderSelectionChanged(0L);
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
			folderSelectionChanged(0L); //_ww->_folderView->currentItem());
		}
	}
}


void KWalletEditor::createFolder() {
	if (_w) {
		QString n;

		do {
			n = KLineEditDlg::getText(i18n("New Folder..."),
					i18n("Please choose a name for the new folder..."),
					QString::null,
					0L,
					this);

			if (n.isEmpty()) {
				return;
			}

			if (_ww->_folderView->findItem(n)) {
				int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that folder name is in use.  Try again?"));
				if (rc == KMessageBox::Yes) {
					continue;
				}
				n = QString::null;
			}
			break;
		} while (true);

		_w->createFolder(n);
		updateFolderList();
	}
}


void KWalletEditor::saveEntry() {
	int rc = 1;
	QListViewItem *item = _ww->_entryList->currentItem();
	_ww->_saveChanges->setEnabled(false);
	_ww->_undoChanges->setEnabled(false);

	if (item && _w && item->parent()) {
		if (item->parent() == _passItems) {
			rc = _w->writePassword(item->text(0), _ww->_passwordValue->text());
			if (rc == 0) {
				return;
			}
		} else {
			return;
		}
	}

	KMessageBox::sorry(this, i18n("Error saving entry.  Error code: %1").arg(rc));
}


void KWalletEditor::restoreEntry() {
	entrySelectionChanged(_ww->_entryList->currentItem());
}


void KWalletEditor::entryEditted() {
	_ww->_saveChanges->setEnabled(true);
	_ww->_undoChanges->setEnabled(true);
}


void KWalletEditor::entrySelectionChanged(QListViewItem *item) {
	_ww->_saveChanges->setEnabled(false);
	_ww->_undoChanges->setEnabled(false);

	if (item && _w && item->parent()) {
		if (item->parent() == _passItems) {
			QString pass;
			if (_w->readPassword(item->text(0), pass) == 0) {
				_ww->_entryStack->raiseWidget(int(1));
				_ww->_entryName->setText(i18n("Password: %1")
							.arg(item->text(0)));
				_ww->_passwordValue->setText(pass);
				_ww->_saveChanges->setEnabled(false);
				_ww->_undoChanges->setEnabled(false);
				return;
			}
		} else if (item->parent() == _mapItems) {
			_ww->_entryStack->raiseWidget(int(2));
			QMap<QString,QString> map;
			if (_w->readMap(item->text(0), map) == 0) {
				_ww->_entryName->setText(i18n("Name-Value Map: %1").arg(item->text(0)));
				_ww->_saveChanges->setEnabled(false);
				_ww->_undoChanges->setEnabled(false);
			}
		} else if (item->parent() == _binaryItems) {
			_ww->_entryStack->raiseWidget(int(3));
			QByteArray ba;
			if (_w->readEntry(item->text(0), ba) == 0) {
				_ww->_entryName->setText(i18n("Binary Data: %1")
							.arg(item->text(0)));
				_ww->_saveChanges->setEnabled(false);
				_ww->_undoChanges->setEnabled(false);
			}
		}
	}

	_ww->_entryName->clear();
	_ww->_entryStack->raiseWidget(int(0));
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
		_details->write(QString::null);
		_details->end();
		_ww->_entryList->clear();
		entrySelectionChanged(0L);
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
	_passItems = new QListViewItem(_ww->_entryList, i18n("Passwords"));
	_mapItems = new QListViewItem(_ww->_entryList, i18n("Maps"));
	_binaryItems = new QListViewItem(_ww->_entryList, i18n("Binary Data"));
	_unknownItems = new QListViewItem(_ww->_entryList, i18n("Unknown"));
	
	for (QStringList::Iterator i = _entries.begin(); i != _entries.end(); ++i) {
		switch (_w->entryType(*i)) {
		case KWallet::Wallet::Password:
			new KWalletEntryItem(_passItems, *i);
			break;
		case KWallet::Wallet::Stream:
			new KWalletEntryItem(_mapItems, *i);
			break;
		case KWallet::Wallet::Map:
			new KWalletEntryItem(_binaryItems, *i);
			break;
		case KWallet::Wallet::Unknown:
		default:
			new QListViewItem(_unknownItems, *i);
			break;
		}
	}
}


void KWalletEditor::iconContextMenuRequested(QIconViewItem *item, const QPoint& pos) {
	KPopupMenu *m = new KPopupMenu(this);
	if (item) {
		m->insertTitle(item->text());
		_newFolderAction->plug(m);
		_deleteFolderAction->plug(m);
		m->popup(pos);
	} else {
		_newFolderAction->plug(m);
		m->popup(pos);
	}
}


void KWalletEditor::listContextMenuRequested(QListViewItem *item, const QPoint& pos, int col) {
	Q_UNUSED(col)

	if (!item || item == _unknownItems ||
			(item->parent() && item->parent() == _unknownItems)) {
		return;
	}

	KPopupMenu *m = new KPopupMenu(this);
	m->insertTitle(item->text(0));
	if (item->parent()) {
		m->insertItem("&New...", this, SLOT(newEntry()), Key_Insert);
		m->insertItem("&Rename", this, SLOT(renameEntry()), Key_F2);
		m->insertItem("&Delete", this, SLOT(deleteEntry()), Key_Delete);
		m->popup(pos);
	} else {
		m->insertItem("&New...", this, SLOT(newEntry()), Key_Insert);
		m->popup(pos);
	}
}


void KWalletEditor::newEntry() {
QListViewItem *item = _ww->_entryList->selectedItem();
QString n;

	do {
		n = KLineEditDlg::getText(i18n("New Entry..."),
				i18n("Please choose a name for the new entry..."),
				QString::null,
				0L,
				this);

		if (n.isEmpty()) {
			return;
		}

		// FIXME: prohibits the use of the subheadings
		if (_ww->_entryList->findItem(n, 0)) {
			int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that entry already exists.  Try again?"));
			if (rc == KMessageBox::Yes) {
				continue;
			}
			n = QString::null;
		}
		break;
	} while (true);

	if (_w && item && !n.isEmpty()) {
		QListViewItem *p = item;
		if (item->parent()) {
			p = item->parent();
		}

		new KWalletEntryItem(p, n);

		if (p == _passItems) {
			_w->writePassword(n, QString::null);
		} else if (p == _mapItems) {
			_w->writeMap(n, QMap<QString,QString>());
		} else if (p == _binaryItems) {
			_w->writeEntry(n, QByteArray());
		} else {
			assert(0);
		}
	}
}


void KWalletEditor::renameEntry() {
QListViewItem *item = _ww->_entryList->selectedItem();
	if (_w && item) {
		item->startRename(0);
	}
}


// Only supports renaming of KWalletEntryItem derived classes.
void KWalletEditor::listItemRenamed(QListViewItem* item, int, const QString& t) {
	if (item) {
		KWalletEntryItem *i = dynamic_cast<KWalletEntryItem*>(item);
		if (!i) {
			return;
		}

		if (!_w) {
			i->setText(0, i->oldName());
			return;
		}

		if (_w->renameEntry(i->oldName(), t) == 0) {
			i->clearOldName();
		} else {
			i->setText(0, i->oldName());
		}
	}
}


void KWalletEditor::deleteEntry() {
QListViewItem *item = _ww->_entryList->selectedItem();
	if (_w && item) {
		_w->removeEntry(item->text(0));
		delete item;
	}
}


void KWalletEditor::updateEntries(const QString& folder) {
QIconViewItem *ivi = _ww->_folderView->currentItem();

	if (ivi && ivi->text() == folder) {
		// FIXME - this is sort of destructive
		_entries = _w->entryList();
		updateEntries();
		updateDetails();
	}
}

#include "kwalleteditor.moc"

