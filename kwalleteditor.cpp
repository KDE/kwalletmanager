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
#include "kwmapeditor.h"
#include "allyourbase.h"

#include <dcopclient.h>
#include <dcopref.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kiconview.h>
#include <kinputdialog.h>
#include <kio/netaccess.h>
#include <klistviewsearchline.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <ksqueezedtextlabel.h>
#include <kstdaction.h>
#include <kstringhandler.h>
#include <ktempfile.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qptrstack.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qwidgetstack.h>

#include <assert.h>

KWalletEditor::KWalletEditor(const QString& wallet, bool isPath, QWidget *parent, const char *name)
: KMainWindow(parent, name), _walletName(wallet), _nonLocal(isPath) {
	_ww = new WalletWidget(this, "Wallet Widget");
	QVBoxLayout *box = new QVBoxLayout(_ww->_folderDetails);
	_details = new QLabel(_ww->_folderDetails, "Folder Details");
        _details->setBackgroundMode(PaletteBase);
        _details->setAlignment((_details->alignment() & ~AlignVertical_Mask)|AlignTop);
        _details->setFrameStyle(QFrame::Sunken | QFrame::Panel);
	box->addWidget(_details);

	box = new QVBoxLayout(_ww->_entryListFrame);
	_entryList = new KWalletEntryList(_ww->_entryListFrame, "Wallet Entry List");
	box->addWidget(new KListViewSearchLine(_ww->_entryListFrame, _entryList));
	box->addWidget(_entryList);

	_entryList->setEnabled(false);
	_ww->_folderDetails->setEnabled(false);
	_ww->_entryStack->setEnabled(false);

	box = new QVBoxLayout(_ww->_folderViewFrame);
	_folderView = new KWalletFolderIconView(_ww->_folderViewFrame, "Wallet Folder View");
	box->addWidget(_folderView);
	_folderView->_walletName = wallet;

	box = new QVBoxLayout(_ww->_entryStack->widget(2));
	_mapEditorShowHide = new QCheckBox(i18n("&Show values"), _ww->_entryStack->widget(2));
	connect(_mapEditorShowHide, SIGNAL(toggled(bool)), this, SLOT(showHideMapEditorValue(bool)));
	_mapEditor = new KWMapEditor(_currentMap, _ww->_entryStack->widget(2));
	box->addWidget(_mapEditorShowHide);
	box->addWidget(_mapEditor);

	setCentralWidget(_ww);

	// Try to make it look nice
	resize(600, 600);
	QValueList<int> sz = _ww->_topSplitter->sizes();
	int sum = sz[0] + sz[1];
	sz[0] = sum/2;
	sz[1] = sum/2;
	_ww->_topSplitter->setSizes(sz);
	sz[0] = sum*2/5;
	sz[1] = sum*3/5;
	_ww->_bottomSplitter->setSizes(sz);

	sz = _ww->_midSplitter->sizes();
	sum = sz[0] + sz[1];
	sz[0] = sum/3;
	sz[1] = sum*2/3;
	_ww->_midSplitter->setSizes(sz);

	connect(_folderView, SIGNAL(selectionChanged(QIconViewItem*)),
		this, SLOT(folderSelectionChanged(QIconViewItem*)));
	connect(_entryList, SIGNAL(selectionChanged(QListViewItem*)),
		this, SLOT(entrySelectionChanged(QListViewItem*)));
	connect(_entryList,
		SIGNAL(contextMenuRequested(QListViewItem*,const QPoint&,int)),
		this,
		SLOT(listContextMenuRequested(QListViewItem*,const QPoint&,int)));
	connect(_entryList,
		SIGNAL(itemRenamed(QListViewItem*, int, const QString&)),
		this,
		SLOT(listItemRenamed(QListViewItem*, int, const QString&)));
	connect(_folderView,
		SIGNAL(contextMenuRequested(QIconViewItem*,const QPoint&)),
		this,
		SLOT(iconContextMenuRequested(QIconViewItem*,const QPoint&)));

	connect(_ww->_passwordValue, SIGNAL(textChanged()),
		this, SLOT(entryEditted()));
	connect(_mapEditor, SIGNAL(dirty()),
		this, SLOT(entryEditted()));

	connect(_ww->_undoChanges, SIGNAL(clicked()),
		this, SLOT(restoreEntry()));
	connect(_ww->_saveChanges, SIGNAL(clicked()),
		this, SLOT(saveEntry()));

	connect(_ww->_showContents, SIGNAL(clicked()),
		this, SLOT(showPasswordContents()));
	connect(_ww->_hideContents, SIGNAL(clicked()),
		this, SLOT(hidePasswordContents()));

	_passItems = new QListViewItem(_entryList, i18n("Passwords"));
	_mapItems = new QListViewItem(_entryList, i18n("Maps"));
	_binaryItems = new QListViewItem(_entryList, i18n("Binary Data"));
	_unknownItems = new QListViewItem(_entryList, i18n("Unknown"));

	_w = KWallet::Wallet::openWallet(wallet, winId(), isPath ? KWallet::Wallet::Path : KWallet::Wallet::Asynchronous);
	if (_w) {
		connect(_w, SIGNAL(walletOpened(bool)), this, SLOT(walletOpened(bool)));
		connect(_w, SIGNAL(walletClosed()), this, SLOT(walletClosed()));
		connect(_w, SIGNAL(folderUpdated(const QString&)), this, SLOT(updateEntries(const QString&)));
		connect(_w, SIGNAL(folderListUpdated()), this, SLOT(updateFolderList()));
		updateFolderList();
	} else {
		kdDebug(2300) << "Wallet open failed!" << endl;
	}

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
	if (_nonLocal) {
		KWallet::Wallet::closeWallet(_walletName, true);
	}
}

void KWalletEditor::createActions() {
	_newFolderAction = new KAction(i18n("&New Folder..."), "folder_new",
			0, this, SLOT(createFolder()), actionCollection(),
			"create_folder");
	connect(this, SIGNAL(enableFolderActions(bool)),
		_newFolderAction, SLOT(setEnabled(bool)));

	_deleteFolderAction = new KAction(i18n("&Delete Folder"), 0, 0,
			this, SLOT(deleteFolder()), actionCollection(),
			"delete_folder");
	connect(this, SIGNAL(enableContextFolderActions(bool)),
		_deleteFolderAction, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(enableFolderActions(bool)),
		_deleteFolderAction, SLOT(setEnabled(bool)));

	_passwordAction = new KAction(i18n("Change &Password..."), 0, 0, this,
			SLOT(changePassword()), actionCollection(),
			"change_password");

	_exportAction = new KAction(i18n("&Export..."), 0, 0, this,
			SLOT(exportXML()), actionCollection(),
			"export");

	KStdAction::quit(this, SLOT(close()), actionCollection());
	KStdAction::keyBindings(guiFactory(), SLOT(configureShortcuts()),
actionCollection());
	emit enableFolderActions(_w != 0L);
	emit enableContextFolderActions(false);
}


void KWalletEditor::walletClosed() {
	delete _w;
	_w = 0L;
	_folderView->clear();
	folderSelectionChanged(0L);
	_ww->setEnabled(false);
	emit enableFolderActions(false);
	KMessageBox::sorry(this, i18n("This wallet was forced closed.  You must reopen it to continue working with it."));
}


void KWalletEditor::updateFolderList() {
QStringList fl = _w->folderList();
QPtrStack<QIconViewItem> trash;

	for (QIconViewItem *item = _folderView->firstItem(); item; item = item->nextItem()) {
		if (!fl.contains(item->text())) {
			trash.push(item);
		}
	}

	trash.setAutoDelete(true);
	trash.clear();

	for (QStringList::Iterator i = fl.begin(); i != fl.end(); ++i) {
		if (!_folderView->findItem(*i)) {
			new KWalletFolderItem(_w, _folderView, *i);
		}
	}
}


void KWalletEditor::deleteFolder() {
	if (_w) {
		QIconViewItem *ivi = _folderView->currentItem();
		if (ivi) {
			int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the folder '%1' from the wallet?").arg(_folderView->currentItem()->text()),"",KStdGuiItem::del());
			if (rc == KMessageBox::Continue) {
				int rc = _w->removeFolder(ivi->text());
				if (rc != 0) {
					KMessageBox::sorry(this, i18n("Error deleting folder.  Error code=%1").arg(rc));
					return;
				}
				updateFolderList();
				folderSelectionChanged(0L);
			}
		}
	}
}


void KWalletEditor::createFolder() {
	if (_w) {
		QString n;
		bool ok;

		do {
			n = KInputDialog::getText(i18n("New Folder"),
					i18n("Please choose a name for the new folder:"),
					QString::null,
					&ok,
					this);

			if (!ok) {
				return;
			}

			if (_folderView->findItem(n)) {
				int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that folder name is in use. Try again?"));
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
	QListViewItem *item = _entryList->currentItem();
	_ww->_saveChanges->setEnabled(false);
	_ww->_undoChanges->setEnabled(false);

	if (item && _w && item->parent()) {
		if (item->parent() == _passItems) {
			rc = _w->writePassword(item->text(0), _ww->_passwordValue->text());
		} else if (item->parent() == _mapItems) {
			_mapEditor->saveMap();
			rc = _w->writeMap(item->text(0), _currentMap);
		} else {
			return;
		}

		if (rc == 0) {
			return;
		}
	}

	KMessageBox::sorry(this, i18n("Error saving entry. Error code: %1").arg(rc));
}


void KWalletEditor::restoreEntry() {
	entrySelectionChanged(_entryList->currentItem());
}


void KWalletEditor::entryEditted() {
	_ww->_saveChanges->setEnabled(true);
	_ww->_undoChanges->setEnabled(true);
}


void KWalletEditor::entrySelectionChanged(QListViewItem *item) {
	if (item && _w && item->parent()) {
		if (item->parent() == _passItems) {
			QString pass;
			if (_w->readPassword(item->text(0), pass) == 0) {
				_ww->_entryStack->raiseWidget(int(4));
				_ww->_entryName->setText(i18n("Password: %1")
							.arg(item->text(0)));
				_ww->_passwordValue->setText(pass);
				_ww->_saveChanges->setEnabled(false);
				_ww->_undoChanges->setEnabled(false);
				return;
			}
		} else if (item->parent() == _mapItems) {
			_ww->_entryStack->raiseWidget(int(2));
			_mapEditorShowHide->setChecked(false);
			showHideMapEditorValue(false);
			if (_w->readMap(item->text(0), _currentMap) == 0) {
				_mapEditor->reload();
				_ww->_entryName->setText(i18n("Name-Value Map: %1").arg(item->text(0)));
				_ww->_saveChanges->setEnabled(false);
				_ww->_undoChanges->setEnabled(false);
				return;
			}
		} else if (item->parent() == _binaryItems) {
			_ww->_entryStack->raiseWidget(int(3));
			QByteArray ba;
			if (_w->readEntry(item->text(0), ba) == 0) {
				_ww->_entryName->setText(i18n("Binary Data: %1")
							.arg(item->text(0)));
				_ww->_saveChanges->setEnabled(false);
				_ww->_undoChanges->setEnabled(false);
				return;
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
		_details->setText(QString::null);
		while (_passItems->firstChild()) {
			delete _passItems->firstChild();
		}
		while (_mapItems->firstChild()) {
			delete _mapItems->firstChild();
		}
		while (_binaryItems->firstChild()) {
			delete _binaryItems->firstChild();
		}
		while (_unknownItems->firstChild()) {
			delete _unknownItems->firstChild();
		}
		entrySelectionChanged(0L);
	}
	_entryList->setEnabled(item && _w);
	_ww->_folderDetails->setEnabled(item && _w);
	_ww->_entryName->setText(QString::null);
	_ww->_entryStack->raiseWidget(int(0));
	_ww->_entryStack->setEnabled(item && _w);
}


void KWalletEditor::updateDetails() {
	static const QString page = i18n("<qt><br /><center>"
			"<table width=\"90%\"><tr><td bgcolor=\"#99ccff\">"
			"<font size=\"+2\">&nbsp;%1</font>"
			"&nbsp;&nbsp;<font size=\"+1\"><b>%2</b>"
                        "</font></td></tr></table></center>"
			"<ul>"
			"<li>%3</li>"
			"</ul>"
			"</qt>");

	_details->setText(page
                        .arg(i18n("Folder:"))
			.arg(_folderView->currentItem()->text())
			.arg(i18n("Contains one item.", "Contains %n items." ,_entries.count())));
}


void KWalletEditor::updateEntries() {
QPtrStack<QListViewItem> trash;

	// Remove deleted entries
	for (QListViewItem *i = _passItems->firstChild(); i; i = i->nextSibling()) {
		if (!_entries.contains(i->text(0))) {
			if (i == _entryList->currentItem()) {
				entrySelectionChanged(0L);
			}
			trash.push(i);
		}
	}

	for (QListViewItem *i = _mapItems->firstChild(); i; i = i->nextSibling()) {
		if (!_entries.contains(i->text(0))) {
			if (i == _entryList->currentItem()) {
				entrySelectionChanged(0L);
			}
			trash.push(i);
		}
	}

	for (QListViewItem *i = _binaryItems->firstChild(); i; i = i->nextSibling()) {
		if (!_entries.contains(i->text(0))) {
			if (i == _entryList->currentItem()) {
				entrySelectionChanged(0L);
			}
			trash.push(i);
		}
	}

	for (QListViewItem *i = _unknownItems->firstChild(); i; i = i->nextSibling()) {
		if (!_entries.contains(i->text(0))) {
			if (i == _entryList->currentItem()) {
				entrySelectionChanged(0L);
			}
			trash.push(i);
		}
	}

	trash.setAutoDelete(true);
	trash.clear();

	// Add new entries
	for (QStringList::Iterator i = _entries.begin(); i != _entries.end(); ++i) {
		QListViewItem *si = _entryList->findItem(*i, 0);
		if (si && si->parent()) {
			continue;
		}

		switch (_w->entryType(*i)) {
		case KWallet::Wallet::Password:
			new KWalletEntryItem(_w, _passItems, *i);
			break;
		case KWallet::Wallet::Stream:
			new KWalletEntryItem(_w, _binaryItems, *i);
			break;
		case KWallet::Wallet::Map:
			new KWalletEntryItem(_w, _mapItems, *i);
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
	QString title = item->text(0);
	// I think 200 pixels is wide enough for a title
	title = KStringHandler::cPixelSqueeze(title, m->fontMetrics(), 200);
	m->insertTitle(title);
	if (item->parent()) {
		m->insertItem(i18n("&New..." ), this, SLOT(newEntry()), Key_Insert);
		m->insertItem(i18n( "&Rename" ), this, SLOT(renameEntry()), Key_F2);
		m->insertItem(i18n( "&Delete" ), this, SLOT(deleteEntry()), Key_Delete);
		m->popup(pos);
	} else {
		m->insertItem(i18n( "&New..." ), this, SLOT(newEntry()), Key_Insert);
		m->popup(pos);
	}
}


void KWalletEditor::newEntry() {
	QListViewItem *item = _entryList->selectedItem();
	QString n;
	bool ok;

	do {
		n = KInputDialog::getText(i18n("New Entry"),
				i18n("Please choose a name for the new entry:"),
				QString::null,
				&ok,
				this);

		if (!ok) {
			return;
		}

		// FIXME: prohibits the use of the subheadings
		if (_entryList->findItem(n, 0)) {
			int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that entry already exists. Try again?"));
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

		new KWalletEntryItem(_w, p, n);

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
QListViewItem *item = _entryList->selectedItem();
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

		if (!_w || t.isEmpty()) {
			i->setText(0, i->oldName());
			return;
		}

		if (_w->renameEntry(i->oldName(), t) == 0) {
			i->clearOldName();
			QListViewItem *p = item->parent();
			if (p == _passItems) {
				_ww->_entryName->setText(i18n("Password: %1").arg(item->text(0)));
			} else if (p == _mapItems) {
				_ww->_entryName->setText(i18n("Name-Value Map: %1").arg(item->text(0)));
			} else if (p == _binaryItems) {
				_ww->_entryName->setText(i18n("Binary Data: %1").arg(item->text(0)));
			}
		} else {
			i->setText(0, i->oldName());
		}
	}
}


void KWalletEditor::deleteEntry() {
QListViewItem *item = _entryList->selectedItem();
	if (_w && item) {
		int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the item '%1'?").arg(item->text(0)),"",KStdGuiItem::del());
		if (rc == KMessageBox::Continue) {
			_w->removeEntry(item->text(0));
			delete item;
			entrySelectionChanged(_entryList->currentItem());
		}
	}
}


void KWalletEditor::updateEntries(const QString& folder) {
QIconViewItem *ivi = _folderView->currentItem();

	if (ivi && ivi->text() == folder) {
		_entries = _w->entryList();
		updateEntries();
		updateDetails();
	}
}


void KWalletEditor::changePassword() {
	KWallet::Wallet::changePassword(_walletName);
}


void KWalletEditor::walletOpened(bool success) {
	if (success) {
		updateFolderList();
	} else {
		KMessageBox::sorry(this, i18n("Unable to open the requested wallet."));
	}
}


void KWalletEditor::hidePasswordContents() {
	_ww->_entryStack->raiseWidget(int(4));
}


void KWalletEditor::showPasswordContents() {
	_ww->_entryStack->raiseWidget(int(1));
}


void KWalletEditor::showHideMapEditorValue(bool show) {
	if (show) {
		_mapEditor->showColumn(2);
	} else {
		_mapEditor->hideColumn(2);
	}
}


void KWalletEditor::exportXML() {
	KTempFile tf;
	tf.setAutoDelete(true);
	QTextStream& ts(*tf.textStream());
	QStringList fl = _w->folderList();

	ts << "<wallet name=\"" << _walletName << "\">" << endl;
	for (QStringList::Iterator i = fl.begin(); i != fl.end(); ++i) {
		ts << "  <folder name=\"" << *i << "\">" << endl;
		_w->setFolder(*i);
		QStringList entries = _w->entryList();
		for (QStringList::Iterator j = entries.begin(); j != entries.end(); ++j) {
			switch (_w->entryType(*j)) {
				case KWallet::Wallet::Password:
					{
						QString pass;
						if (_w->readPassword(*j, pass) == 0) {
							ts << "    <password name=\"" << QStyleSheet::escape(*j) << "\">";
							ts << QStyleSheet::escape(pass);
							ts << "</password>" << endl;
						}
						break;
					}
				case KWallet::Wallet::Stream:
					{
						QByteArray ba;
						if (_w->readEntry(*j, ba) == 0) {
							ts << "    <stream name=\"" << QStyleSheet::escape(*j) << "\">";
							ts << KCodecs::base64Encode(ba);

							ts << "</stream>" << endl;
						}
						break;
					}
				case KWallet::Wallet::Map:
					{
						QMap<QString,QString> map;
						if (_w->readMap(*j, map) == 0) {
							ts << "    <map name=\"" << QStyleSheet::escape(*j) << "\">" << endl;
							for (QMap<QString,QString>::ConstIterator k = map.begin(); k != map.end(); ++k) {
								ts << "      <mapentry name=\"" << QStyleSheet::escape(k.key()) << "\">" << QStyleSheet::escape(k.data()) << "</mapentry>" << endl;
							}
							ts << "    </map>" << endl;
						}
						break;
					}
				case KWallet::Wallet::Unknown:
				default:
					break;
			}
		}
		ts << "  </folder>" << endl;
	}

	ts << "</wallet>" << endl;
	tf.close();

	KURL url = KFileDialog::getSaveURL(QString::null, "*.xml", this);

	if (!url.isEmpty()) {
		KIO::NetAccess::file_copy(tf.name(), url, 0600, false, false, this);
	}
}


#include "kwalleteditor.moc"

