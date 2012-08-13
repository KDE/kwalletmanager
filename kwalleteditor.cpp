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

#include "kwalleteditor.h"
#include "kbetterthankdialogbase.h"
#include "kwmapeditor.h"
#include "allyourbase.h"

#include <stdlib.h>
#include <QDomElement>
#include <QDomNode>
#include <QDomDocument>
#include <QXmlStreamWriter>
#include <kaction.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <kio/netaccess.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <kstandardaction.h>

#include <ktemporaryfile.h>
#include <kxmlguifactory.h>
#include <QCheckBox>
#include <QClipboard>
#include <QPushButton>
#include <QTimer>
#include <QSet>
#include <QTextStream>
#include <QList>
#include <QVBoxLayout>
#include <QStack>

#include <assert.h>
#include <ktoolbar.h>
#include <kicon.h>
#include <KAction>
#include <KTreeWidgetSearchLine>

KWalletEditor::KWalletEditor(const QString& wallet, bool isPath, QWidget *parent, const char *name)
: KXmlGuiWindow(parent), _walletName(wallet), _nonLocal(isPath), _displayedItem(0)  {
	setObjectName( QLatin1String( name ) );
	_newWallet = false;
	_ww = new WalletWidget(this);
	_ww->_splitter->setStretchFactor(0, 1);
	_ww->_splitter->setStretchFactor(1, 2);
	_contextMenu = new KMenu(this);

	_ww->_undoChanges->setIcon(KIcon( QLatin1String( "edit-undo" )));
	_ww->_saveChanges->setIcon(KIcon( QLatin1String( "document-save" )));

	QVBoxLayout *box = new QVBoxLayout(_ww->_entryListFrame);
	box->setSpacing( KDialog::spacingHint() );
	box->setMargin( KDialog::marginHint() );
	_entryList = new KWalletEntryList(_ww->_entryListFrame, "Wallet Entry List");
	_entryList->setContextMenuPolicy(Qt::CustomContextMenu);
	box->addWidget(new KTreeWidgetSearchLine(_ww->_entryListFrame, _entryList));
	box->addWidget(_entryList);

	_ww->_entryStack->setEnabled(true);

	box = new QVBoxLayout(_ww->_entryStack->widget(2));
	_mapEditorShowHide = new QCheckBox(i18n("&Show values"), _ww->_entryStack->widget(2));
	connect(_mapEditorShowHide, SIGNAL(toggled(bool)), this, SLOT(showHideMapEditorValue(bool)));
	_mapEditor = new KWMapEditor(_currentMap, _ww->_entryStack->widget(2));
	box->addWidget(_mapEditorShowHide);
	box->addWidget(_mapEditor);

	setCentralWidget(_ww);
	setAutoSaveSettings(QLatin1String("WalletEditor"), true);

	// load splitter size
	KConfigGroup cg(KGlobal::config(), "WalletEditor");
	QList<int> splitterSize = cg.readEntry("SplitterSize", QList<int>());
	if (splitterSize.size() != 2) {
		splitterSize.clear();
		splitterSize.append(_ww->_splitter->width()/2);
		splitterSize.append(_ww->_splitter->width()/2);
	}
	_ww->_splitter->setSizes(splitterSize);

	connect(_entryList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
		this, SLOT(entrySelectionChanged(QTreeWidgetItem*)));
	connect(_entryList,
		SIGNAL(customContextMenuRequested(QPoint)),
		this,
		SLOT(listContextMenuRequested(QPoint)));
	connect(_entryList,
		SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		this,
		SLOT(listItemChanged(QTreeWidgetItem*,int)));

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

	_walletIsOpen = false;

	_w = KWallet::Wallet::openWallet(wallet, winId(), isPath ? KWallet::Wallet::Path : KWallet::Wallet::Asynchronous);
	if (_w) {
		connect(_w, SIGNAL(walletOpened(bool)), this, SLOT(walletOpened(bool)));
		connect(_w, SIGNAL(walletClosed()), this, SLOT(walletClosed()));
		connect(_w, SIGNAL(folderUpdated(QString)), this, SLOT(updateEntries(QString)));
		connect(_w, SIGNAL(folderListUpdated()), this, SLOT(updateFolderList()));
		updateFolderList();
	} else {
		kDebug() << "Wallet open failed!" ;
	}

	createActions();
	createGUI( QLatin1String( "kwalleteditor.rc" ));
	delete toolBar();

	setCaption(wallet);
}

KWalletEditor::~KWalletEditor() {
	// save splitter size
	KConfigGroup cg(KGlobal::config(), "WalletEditor");
	cg.writeEntry("SplitterSize", _ww->_splitter->sizes());
	cg.sync();

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
        _newFolderAction = actionCollection()->addAction( QLatin1String( "create_folder" ));
        _newFolderAction->setText( i18n("&New Folder...") );
        _newFolderAction->setIcon( KIcon( QLatin1String( "folder-new" )) );
	connect(_newFolderAction, SIGNAL(triggered(bool)), SLOT(createFolder()));
	connect(this, SIGNAL(enableFolderActions(bool)),
		_newFolderAction, SLOT(setEnabled(bool)));

        _deleteFolderAction = actionCollection()->addAction( QLatin1String( "delete_folder" ) );
        _deleteFolderAction->setText( i18n("&Delete Folder") );
	connect(_deleteFolderAction, SIGNAL(triggered(bool)), SLOT(deleteFolder()));
	connect(this, SIGNAL(enableContextFolderActions(bool)),
		_deleteFolderAction, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(enableFolderActions(bool)),
		_deleteFolderAction, SLOT(setEnabled(bool)));

        _passwordAction = actionCollection()->addAction( QLatin1String(  "change_password" ) );
        _passwordAction->setText( i18n("Change &Password...") );
	connect(_passwordAction, SIGNAL(triggered(bool)), SLOT(changePassword()));
	connect(this, SIGNAL(enableWalletActions(bool)),
		_passwordAction, SLOT(setEnabled(bool)));

        _mergeAction = actionCollection()->addAction( QLatin1String(  "merge" ));
        _mergeAction->setText( i18n("&Merge Wallet...") );
	connect(_mergeAction, SIGNAL(triggered(bool)), SLOT(importWallet()));
	connect(this, SIGNAL(enableWalletActions(bool)),
		_mergeAction, SLOT(setEnabled(bool)));

        _importAction= actionCollection()->addAction( QLatin1String(  "import" ) );
        _importAction->setText( i18n("&Import XML...") );
	connect(_importAction, SIGNAL(triggered(bool)), SLOT(importXML()));
	connect(this, SIGNAL(enableWalletActions(bool)),
		_importAction, SLOT(setEnabled(bool)));

        _exportAction = actionCollection()->addAction( QLatin1String(  "export" ) );
        _exportAction->setText( i18n("&Export...") );
	connect(_exportAction, SIGNAL(triggered(bool)), SLOT(exportXML()));
	connect(this, SIGNAL(enableWalletActions(bool)),
		_exportAction, SLOT(setEnabled(bool)));

	_saveAsAction = KStandardAction::saveAs(this, SLOT(saveAs()), actionCollection());
	connect(this, SIGNAL(enableWalletActions(bool)),
		_saveAsAction, SLOT(setEnabled(bool)));

	_copyPassAction = KStandardAction::copy(this, SLOT(copyPassword()), actionCollection());

	_newEntryAction = actionCollection()->addAction( QLatin1String(  "new_entry" ) );
	_newEntryAction->setText( i18n( "&New..." ) );
	_newEntryAction->setShortcut( Qt::Key_Insert );
	connect(_newEntryAction, SIGNAL(triggered(bool)), SLOT(newEntry()));
	_newEntryAction->setEnabled(false);

	_renameEntryAction = actionCollection()->addAction( QLatin1String(  "rename_entry" ) );
	_renameEntryAction->setText( i18n( "&Rename" ) );
	_renameEntryAction->setShortcut( Qt::Key_F2 );
	connect(_renameEntryAction, SIGNAL(triggered(bool)), SLOT(renameEntry()));
	_renameEntryAction->setEnabled(false);

	_deleteEntryAction = actionCollection()->addAction( QLatin1String(  "delete_entry" ) );
	_deleteEntryAction->setText( i18n( "&Delete" ) );
	_deleteEntryAction->setShortcut( Qt::Key_Delete );
	connect(_deleteEntryAction, SIGNAL(triggered(bool)), SLOT(deleteEntry()));
	_deleteEntryAction->setEnabled(false);

	KStandardAction::quit(this, SLOT(close()), actionCollection());
	KStandardAction::keyBindings(guiFactory(), SLOT(configureShortcuts()),
actionCollection());
	emit enableWalletActions(false);
	emit enableFolderActions(false);
	emit enableContextFolderActions(false);
}


void KWalletEditor::walletClosed() {
	delete _w;
	_walletIsOpen = false;
	_w = 0L;
	_ww->setEnabled(false);
	emit enableWalletActions(false);
	emit enableFolderActions(false);
	this->hide();
	KMessageBox::sorry(this, i18n("This wallet was forced closed.  You must reopen it to continue working with it."));
	deleteLater();
}


void KWalletEditor::updateFolderList(bool checkEntries) {
	const QStringList fl = _w->folderList();
	QStack<QTreeWidgetItem*> trash;

	for (int i = 0; i < _entryList->topLevelItemCount(); ++i) {
		KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem *>(_entryList->topLevelItem(i));
		if (!fi) {
			continue;
		}
		if (!fl.contains(fi->name())) {
			trash.push(fi);
		}
	}

	qDeleteAll(trash);
	trash.clear();

	for (QStringList::const_iterator i = fl.begin(); i != fl.end(); ++i) {
		if (_entryList->existsFolder(*i)) {
			if (checkEntries) {
				updateEntries(*i);
			}
			continue;
		}

		_w->setFolder(*i);
		const QStringList entries = _w->entryList();
		KWalletFolderItem *item = new KWalletFolderItem(_w,_entryList, *i, entries.count());

		KWalletContainerItem *pi = new KWalletContainerItem(item, i18n("Passwords"),KWallet::Wallet::Password);
		KWalletContainerItem *mi = new KWalletContainerItem(item, i18n("Maps"),KWallet::Wallet::Map);
		KWalletContainerItem *bi = new KWalletContainerItem(item, i18n("Binary Data"),KWallet::Wallet::Stream);
		KWalletContainerItem *ui = new KWalletContainerItem(item, i18n("Unknown"),KWallet::Wallet::Unknown);

		for (QStringList::const_iterator j = entries.begin(); j != entries.end(); ++j) {
			switch (_w->entryType(*j)) {
				case KWallet::Wallet::Password:
					new KWalletEntryItem(_w, pi, *j);
					break;
				case KWallet::Wallet::Stream:
					new KWalletEntryItem(_w, bi, *j);
					break;
				case KWallet::Wallet::Map:
					new KWalletEntryItem(_w, mi, *j);
					break;
				case KWallet::Wallet::Unknown:
				default:
					new QTreeWidgetItem(ui, QStringList() << *j);
					break;
			}
		}
		_entryList->setEnabled(true);
	}

	//check if the current folder has been removed
	if (!fl.contains(_currentFolder)) {
		_currentFolder.clear();
		_ww->_entryTitle->clear();
		_ww->_iconTitle->clear();
	}
}

void KWalletEditor::deleteFolder() {
	if (_w) {
		QTreeWidgetItem *i = _entryList->currentItem();
		if (i) {
			KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem *>(i);
			if (!fi) {
				return;
			}

			int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the folder '%1' from the wallet?", fi->name()),QString(),KStandardGuiItem::del());
			if (rc == KMessageBox::Continue) {
				bool rc = _w->removeFolder(fi->name());
				if (!rc) {
					KMessageBox::sorry(this, i18n("Error deleting folder."));
					return;
				}
				_currentFolder.clear();
				_ww->_entryTitle->clear();
				_ww->_iconTitle->clear();
				updateFolderList();
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
					QString(),
					&ok,
					this);

			if (!ok) {
				return;
			}

			if (_entryList->existsFolder(n)) {
				int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that folder name is in use. Try again?"), QString(), KGuiItem(i18n("Try Again")), KGuiItem(i18n("Do Not Try")));
				if (rc == KMessageBox::Yes) {
					continue;
				}
				n.clear();
			}
			break;
		} while (true);

		_w->createFolder(n);
		updateFolderList();
	}
}

void KWalletEditor::saveEntry() {
	int rc = 1;
	QTreeWidgetItem *item = _displayedItem; //  _entryList->currentItem();
	_ww->_saveChanges->setEnabled(false);
	_ww->_undoChanges->setEnabled(false);

	if (item && _w && item->parent()) {
		KWalletContainerItem *ci = dynamic_cast<KWalletContainerItem*>(item->parent());
		if (ci) {
			if (ci->entryType() == KWallet::Wallet::Password) {
				rc = _w->writePassword(item->text(0), _ww->_passwordValue->toPlainText());
			} else if (ci->entryType() == KWallet::Wallet::Map) {
				_mapEditor->saveMap();
				rc = _w->writeMap(item->text(0), _currentMap);
			} else {
				return;
			}

			if (rc == 0) {
				return;
			}
		}
	}

	KMessageBox::sorry(this, i18n("Error saving entry. Error code: %1", rc));
}


void KWalletEditor::restoreEntry() {
	entrySelectionChanged(_entryList->currentItem());
}


void KWalletEditor::entryEditted() {
	_ww->_saveChanges->setEnabled(true);
	_ww->_undoChanges->setEnabled(true);
}


void KWalletEditor::entrySelectionChanged(QTreeWidgetItem *item) {
    // do not forget to save changes
    if ( _ww->_saveChanges->isEnabled() && (_displayedItem != item) ){
        if ( KMessageBox::Yes ==  KMessageBox::questionYesNo(this, 
                                            i18n("The contents of the current item has changed.\nDo you want to save changes?"))) {
            saveEntry();
        } else {
            _ww->_saveChanges->setEnabled(false);
            _ww->_undoChanges->setEnabled(false);
        }
    }
	KWalletContainerItem *ci = 0L;
	KWalletFolderItem *fi = 0L;

	// clear the context menu
	_contextMenu->clear();
	_contextMenu->setEnabled(true);
	// disable the entry actions (reenable them on adding)
	_newEntryAction->setEnabled(false);
	_renameEntryAction->setEnabled(false);
	_deleteEntryAction->setEnabled(false);

	if (item)
	{
		// set the context menu's title
		_contextMenu->addTitle(_contextMenu->fontMetrics().elidedText(
			item->text(0), Qt::ElideMiddle, 200 ) );

		// TODO rtti
		switch (item->type()) {
			case KWalletEntryItemClass:
				ci = dynamic_cast<KWalletContainerItem*>(item->parent());
				if (!ci) {
					return;
				}
				fi = dynamic_cast<KWalletFolderItem*>(ci->parent());
				if (!fi) {
					return;
				}
				_w->setFolder(fi->name());
				_deleteFolderAction->setEnabled(false);

				// add standard menu items
				_contextMenu->addAction( _newEntryAction );
				_contextMenu->addAction( _renameEntryAction );
				_contextMenu->addAction( _deleteEntryAction );
				_newEntryAction->setEnabled(true);
				_renameEntryAction->setEnabled(true);
				_deleteEntryAction->setEnabled(true);

				if (ci->entryType() == KWallet::Wallet::Password) {
					QString pass;
					if (_w->readPassword(item->text(0), pass) == 0) {
						_ww->_entryStack->setCurrentIndex(4);
						_ww->_entryName->setText(i18n("Password: %1",
								item->text(0)));
						_ww->_passwordValue->setText(pass);
						_ww->_saveChanges->setEnabled(false);
						_ww->_undoChanges->setEnabled(false);
					}
					// add a context-menu action for copying passwords
					_contextMenu->addSeparator();
					_contextMenu->addAction( _copyPassAction );
				} else if (ci->entryType() == KWallet::Wallet::Map) {
					_ww->_entryStack->setCurrentIndex(2);
					_mapEditorShowHide->setChecked(false);
					showHideMapEditorValue(false);
					if (_w->readMap(item->text(0), _currentMap) == 0) {
						_mapEditor->reload();
						_ww->_entryName->setText(i18n("Name-Value Map: %1", item->text(0)));
						_ww->_saveChanges->setEnabled(false);
						_ww->_undoChanges->setEnabled(false);
					}
				} else if (ci->entryType() == KWallet::Wallet::Stream) {
					_ww->_entryStack->setCurrentIndex(3);
					QByteArray ba;
					if (_w->readEntry(item->text(0), ba) == 0) {
						_ww->_entryName->setText(i18n("Binary Data: %1",
								item->text(0)));
						_ww->_saveChanges->setEnabled(false);
						_ww->_undoChanges->setEnabled(false);
					}
				}
				break;

			case KWalletContainerItemClass:
				ci = dynamic_cast<KWalletContainerItem*>(item);
				if (!ci) {
					return;
				}
				if (ci->entryType() == KWallet::Wallet::Unknown) {
					// disable context menu on unknown items
					_contextMenu->setEnabled(false);
				} else {
					// add the context menu action
					_contextMenu->addAction( _newEntryAction );
					_newEntryAction->setEnabled(true);
				}

				fi = dynamic_cast<KWalletFolderItem*>(item->parent());
				if (!fi) {
					return;
				}
				_w->setFolder(fi->name());
				_deleteFolderAction->setEnabled(false);
				_ww->_entryName->clear();
				_ww->_entryStack->setCurrentIndex(0);
				break;

			case KWalletFolderItemClass:
				// add the context menu actions
				_contextMenu->addAction( _newFolderAction );
				_contextMenu->addAction( _deleteFolderAction );

				fi = dynamic_cast<KWalletFolderItem*>(item);
				if (!fi) {
					return;
				}
				_w->setFolder(fi->name());
				_deleteFolderAction->setEnabled(true);
				_ww->_entryName->clear();
				_ww->_entryStack->setCurrentIndex(0);
				break;

			default:
				// all items but Unknown entries return have their
				// rtti set. Unknown items can only be deleted.
				_contextMenu->addAction( _deleteEntryAction );
				_deleteEntryAction->setEnabled(true);
				break;
		}
	} else {
		// no item selected. add the "new folder" action to the context menu
		_contextMenu->addAction( _newFolderAction );
	}

	if (fi) {
		_currentFolder = fi->name();
		_ww->_entryTitle->setText(QString::fromLatin1("<font size=\"+1\">%1</font>").arg(fi->text(0)));
		_ww->_iconTitle->setPixmap(fi->getFolderIcon(KIconLoader::Toolbar));
	}
	
	_displayedItem = item;
}

void KWalletEditor::updateEntries(const QString& folder) {
	QStack<QTreeWidgetItem*> trash;

	_w->setFolder(folder);
	const QStringList entries = _w->entryList();

	KWalletFolderItem *fi = _entryList->getFolder(folder);

	if (!fi) {
		return;
	}

	KWalletContainerItem *pi = fi->getContainer(KWallet::Wallet::Password);
	KWalletContainerItem *mi = fi->getContainer(KWallet::Wallet::Map);
	KWalletContainerItem *bi = fi->getContainer(KWallet::Wallet::Stream);
	KWalletContainerItem *ui = fi->getContainer(KWallet::Wallet::Unknown);

	// Remove deleted entries
	for (int i = 0; i < pi->childCount(); ++i) {
		QTreeWidgetItem *twi = pi->child(i);
		if (!entries.contains(twi->text(0))) {
			if (twi == _entryList->currentItem()) {
				entrySelectionChanged(0L);
			}
			trash.push(twi);
		}
	}

	for (int i = 0; i < mi->childCount(); ++i) {
		QTreeWidgetItem *twi = mi->child(i);
		if (!entries.contains(twi->text(0))) {
			if (twi == _entryList->currentItem()) {
				entrySelectionChanged(0L);
			}
			trash.push(twi);
		}
	}

	for (int i = 0; i < bi->childCount(); ++i) {
		QTreeWidgetItem *twi = bi->child(i);
		if (!entries.contains(twi->text(0))) {
			if (twi == _entryList->currentItem()) {
				entrySelectionChanged(0L);
			}
			trash.push(twi);
		}
	}

	for (int i = 0; i < ui->childCount(); ++i) {
		QTreeWidgetItem *twi = ui->child(i);
		if (!entries.contains(twi->text(0))) {
			if (twi == _entryList->currentItem()) {
				entrySelectionChanged(0L);
			}
			trash.push(twi);
		}
	}

	qDeleteAll(trash);
	trash.clear();

	// Add new entries
	for (QStringList::const_iterator i = entries.begin(); i != entries.end(); ++i) {
		if (fi->contains(*i)){
			continue;
		}

		switch (_w->entryType(*i)) {
			case KWallet::Wallet::Password:
				new KWalletEntryItem(_w, pi, *i);
				break;
			case KWallet::Wallet::Stream:
				new KWalletEntryItem(_w, bi, *i);
				break;
			case KWallet::Wallet::Map:
				new KWalletEntryItem(_w, mi, *i);
				break;
			case KWallet::Wallet::Unknown:
			default:
				new QTreeWidgetItem(ui, QStringList() << *i);
				break;
		}
	}
	fi->refresh();
	if (fi->name() == _currentFolder) {
            _ww->_entryTitle->setText(QString::fromLatin1("<font size=\"+1\">%1</font>").arg(fi->text(0)));
	}
	if (!_entryList->currentItem()) {
		_ww->_entryName->clear();
		_ww->_entryStack->setCurrentIndex(0);
	}
}

void KWalletEditor::listContextMenuRequested(const QPoint& pos) {
	if (!_walletIsOpen || !_contextMenu->isEnabled()) {
		return;
	}

	_contextMenu->popup(_entryList->mapToGlobal(pos));
}


void KWalletEditor::copyPassword() {
	QTreeWidgetItem *item = _entryList->currentItem();
	if (_w && item) {
		QString pass;
		if (_w->readPassword(item->text(0), pass) == 0) {
			QApplication::clipboard()->setText(pass);
		}
	}
}


void KWalletEditor::newEntry() {
	QTreeWidgetItem *item = _entryList->currentItem();
	QString n;
	bool ok;

	KWalletFolderItem *fi;

	//set the folder where we're trying to create the new entry
	if (_w && item) {
		QTreeWidgetItem *p = item;
		if (p->type() == KWalletEntryItemClass) {
			p = item->parent();
		}
		fi = dynamic_cast<KWalletFolderItem *>(p->parent());
		if (!fi) {
			return;
		}
		_w->setFolder(fi->name());
	} else {
		return;
	}

	do {
		n = KInputDialog::getText(i18n("New Entry"),
				i18n("Please choose a name for the new entry:"),
				QString(),
				&ok,
				this);

		if (!ok) {
			return;
		}

		// FIXME: prohibits the use of the subheadings
		if (fi->contains(n)) {
			int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that entry already exists. Try again?"), QString(), KGuiItem(i18n("Try Again")), KGuiItem(i18n("Do Not Try")));
			if (rc == KMessageBox::Yes) {
				continue;
			}
			n.clear();
		}
		break;
	} while (true);

	if (_w && item && !n.isEmpty()) {
		QTreeWidgetItem *p = item;
		if (p->type() == KWalletEntryItemClass) {
			p = item->parent();
		}

		KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem *>(p->parent());
		if (!fi) {
			KMessageBox::error(this, i18n("An unexpected error occurred trying to add the new entry"));
			return;
		}
		_w->setFolder(fi->name());

		KWalletEntryItem *ni = new KWalletEntryItem(_w, p, n);

		KWalletContainerItem *ci = dynamic_cast<KWalletContainerItem*>(p);
		if (!ci) {
			KMessageBox::error(this, i18n("An unexpected error occurred trying to add the new entry"));
			delete ni;
			return;
		}
		if (ci->entryType() == KWallet::Wallet::Password) {
			_w->writePassword(n, QString());
		} else if (ci->entryType() == KWallet::Wallet::Map) {
			_w->writeMap(n, QMap<QString,QString>());
		} else if (ci->entryType() == KWallet::Wallet::Stream) {
			_w->writeEntry(n, QByteArray());
		} else {
			abort();
		}

		_entryList->setCurrentItem(ni);
		_entryList->scrollToItem(ni);

		fi->refresh();
		_ww->_entryTitle->setText(QString::fromLatin1("<font size=\"+1\">%1</font>").arg(fi->text(0)));
	}
}


void KWalletEditor::renameEntry() {
	QTreeWidgetItem *item = _entryList->currentItem();
	if (_w && item) {
		_entryList->editItem(item, 0);
	}
}


// Only supports renaming of KWalletEntryItem derived classes.
void KWalletEditor::listItemChanged(QTreeWidgetItem* item, int column) {
	if (item && column == 0) {
		KWalletEntryItem *i = dynamic_cast<KWalletEntryItem*>(item);
		if (!i) {
			return;
		}

		const QString t = item->text(0);
		if (t == i->name()) {
			return;
		}
		if (!_w || t.isEmpty()) {
			i->restoreName();
			return;
		}

		if (_w->renameEntry(i->name(), t) == 0) {
			i->setName(t);
			KWalletContainerItem *ci = dynamic_cast<KWalletContainerItem*>(item->parent());
			if (!ci) {
				KMessageBox::error(this, i18n("An unexpected error occurred trying to rename the entry"));
				return;
			}
			if (ci->entryType() == KWallet::Wallet::Password) {
				_ww->_entryName->setText(i18n("Password: %1", item->text(0)));
			} else if (ci->entryType() == KWallet::Wallet::Map) {
				_ww->_entryName->setText(i18n("Name-Value Map: %1", item->text(0)));
			} else if (ci->entryType() == KWallet::Wallet::Stream) {
				_ww->_entryName->setText(i18n("Binary Data: %1", item->text(0)));
			}
		} else {
			i->restoreName();
		}
	}
}


void KWalletEditor::deleteEntry() {
	QTreeWidgetItem *item = _entryList->currentItem();
	if (_w && item) {
		int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the item '%1'?", item->text(0)),QString(),KStandardGuiItem::del());
		if (rc == KMessageBox::Continue) {
			KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem *>(item->parent()->parent());
			if (!fi) {
				KMessageBox::error(this, i18n("An unexpected error occurred trying to delete the entry"));
				return;
			}
			_w->removeEntry(item->text(0));
			delete item;
			entrySelectionChanged(_entryList->currentItem());
			fi->refresh();
			_ww->_entryTitle->setText(QString::fromLatin1("<font size=\"+1\">%1</font>").arg(fi->text(0)));
		}
	}
}

void KWalletEditor::changePassword() {
	KWallet::Wallet::changePassword(_walletName, winId());
}


void KWalletEditor::walletOpened(bool success) {
	if (success) {
		emit enableFolderActions(true);
		emit enableContextFolderActions(false);
		emit enableWalletActions(true);
		updateFolderList();
		show();
		_entryList->setWallet(_w);
		_walletIsOpen = true;
	} else {
		if (!_newWallet) {
			KMessageBox::sorry(this, i18n("Unable to open the requested wallet."));
		}
		close();
	}
}


void KWalletEditor::hidePasswordContents() {
	_ww->_entryStack->setCurrentIndex(4);
}


void KWalletEditor::showPasswordContents() {
	_ww->_entryStack->setCurrentIndex(1);
}


void KWalletEditor::showHideMapEditorValue(bool show) {
	if (show) {
		_mapEditor->showColumn(2);
	} else {
		_mapEditor->hideColumn(2);
	}
}


enum MergePlan { Prompt = 0, Always = 1, Never = 2, Yes = 3, No = 4 };

void KWalletEditor::importWallet() {
	KUrl url = KFileDialog::getOpenUrl(KUrl(), QLatin1String( "*.kwl" ), this);
	if (url.isEmpty()) {
		return;
	}

	QString tmpFile;
	if (!KIO::NetAccess::download(url, tmpFile, this)) {
		KMessageBox::sorry(this, i18n("Unable to access wallet '<b>%1</b>'.", url.prettyUrl()));
		return;
	}

	KWallet::Wallet *w = KWallet::Wallet::openWallet(tmpFile, winId(), KWallet::Wallet::Path);
	if (w && w->isOpen()) {
		MergePlan mp = Prompt;
		QStringList fl = w->folderList();
		for (QStringList::ConstIterator f = fl.constBegin(); f != fl.constEnd(); ++f) {
			if (!w->setFolder(*f)) {
				continue;
			}

			if (!_w->hasFolder(*f)) {
				_w->createFolder(*f);
			}

			_w->setFolder(*f);

			QMap<QString, QMap<QString, QString> > map;
			QSet<QString> mergedkeys; // prevents re-merging already merged entries.
			int rc;
			rc = w->readMapList(QLatin1String( "*" ), map);
			if (rc == 0) {
				QMap<QString, QMap<QString, QString> >::ConstIterator me;
				for (me = map.constBegin(); me != map.constEnd(); ++me) {
					bool hasEntry = _w->hasEntry(me.key());
					if (hasEntry && mp == Prompt) {
						KBetterThanKDialogBase *bd;
						bd = new KBetterThanKDialogBase(this);
						bd->setLabel(i18n("Folder '<b>%1</b>' already contains an entry '<b>%2</b>'.  Do you wish to replace it?", Qt::escape(*f), Qt::escape(me.key())));
						mp = (MergePlan)bd->exec();
						delete bd;
						bool ok = false;
						if (mp == Always || mp == Yes) {
							ok = true;
						}
						if (mp == Yes || mp == No) {
						       	// reset mp
							mp = Prompt;
						}
						if (!ok) {
							continue;
						}
					} else if (hasEntry && mp == Never) {
						continue;
					}
					_w->writeMap(me.key(), me.value());
					mergedkeys.insert(me.key()); // remember this key has been merged
				}
			}

			QMap<QString, QString> pwd;
			rc = w->readPasswordList(QLatin1String( "*" ), pwd);
			if (rc == 0) {
				QMap<QString, QString>::ConstIterator pe;
				for (pe = pwd.constBegin(); pe != pwd.constEnd(); ++pe) {
					bool hasEntry = _w->hasEntry(pe.key());
					if (hasEntry && mp == Prompt) {
						KBetterThanKDialogBase *bd;
						bd = new KBetterThanKDialogBase(this);
						bd->setLabel(i18n("Folder '<b>%1</b>' already contains an entry '<b>%2</b>'.  Do you wish to replace it?", Qt::escape(*f), Qt::escape(pe.key())));
						mp = (MergePlan)bd->exec();
						delete bd;
						bool ok = false;
						if (mp == Always || mp == Yes) {
							ok = true;
						}
						if (mp == Yes || mp == No) {
						       	// reset mp
							mp = Prompt;
						}
						if (!ok) {
							continue;
						}
					} else if (hasEntry && mp == Never) {
						continue;
					}
					_w->writePassword(pe.key(), pe.value());
					mergedkeys.insert(pe.key()); // remember this key has been merged
				}
			}

			QMap<QString, QByteArray> ent;
			rc = w->readEntryList(QLatin1String( "*" ), ent);
			if (rc == 0) {
				QMap<QString, QByteArray>::ConstIterator ee;
				for (ee = ent.constBegin(); ee != ent.constEnd(); ++ee) {
					// prevent re-merging already merged entries.
					if (mergedkeys.contains(ee.key())) {
						continue;
					}
					bool hasEntry = _w->hasEntry(ee.key());
					if (hasEntry && mp == Prompt) {
						KBetterThanKDialogBase *bd;
						bd = new KBetterThanKDialogBase(this);
						bd->setLabel(i18n("Folder '<b>%1</b>' already contains an entry '<b>%2</b>'.  Do you wish to replace it?", Qt::escape(*f), Qt::escape(ee.key())));
						mp = (MergePlan)bd->exec();
						delete bd;
						bool ok = false;
						if (mp == Always || mp == Yes) {
							ok = true;
						}
						if (mp == Yes || mp == No) {
						       	// reset mp
							mp = Prompt;
						}
						if (!ok) {
							continue;
						}
					} else if (hasEntry && mp == Never) {
						continue;
					}
					_w->writeEntry(ee.key(), ee.value());
				}
			}
		}
	}

	delete w;

	KIO::NetAccess::removeTempFile(tmpFile);
	updateFolderList(true);
	restoreEntry();
}


void KWalletEditor::importXML() {
	KUrl url = KFileDialog::getOpenUrl( KUrl(), QLatin1String( "*.xml" ), this);
	if (url.isEmpty()) {
		return;
	}

	QString tmpFile;
	if (!KIO::NetAccess::download(url, tmpFile, this)) {
		KMessageBox::sorry(this, i18n("Unable to access XML file '<b>%1</b>'.", url.prettyUrl()));
		return;
	}

	QFile qf(tmpFile);
	if (!qf.open(QIODevice::ReadOnly)) {
		KMessageBox::sorry(this, i18n("Error opening XML file '<b>%1</b>' for input.", url.prettyUrl()));
		KIO::NetAccess::removeTempFile(tmpFile);
		return;
	}

	QDomDocument doc(tmpFile);
	if (!doc.setContent(&qf)) {
		KMessageBox::sorry(this, i18n("Error reading XML file '<b>%1</b>' for input.", url.prettyUrl()));
		KIO::NetAccess::removeTempFile(tmpFile);
		return;
	}

	QDomElement top = doc.documentElement();
	if (top.tagName().toLower() != QLatin1String( "wallet" )) {
		KMessageBox::sorry(this, i18n("Error: XML file does not contain a wallet."));
		KIO::NetAccess::removeTempFile(tmpFile);
		return;
	}

	QDomNode n = top.firstChild();
	MergePlan mp = Prompt;
	while (!n.isNull()) {
		QDomElement e = n.toElement();
		if (e.tagName().toLower() != QLatin1String( "folder" )) {
			n = n.nextSibling();
			continue;
		}

		QString fname = e.attribute(QLatin1String( "name" ));
		if (fname.isEmpty()) {
			n = n.nextSibling();
			continue;
		}
		if (!_w->hasFolder(fname)) {
			_w->createFolder(fname);
		}
		_w->setFolder(fname);
		QDomNode enode = e.firstChild();
		while (!enode.isNull()) {
			e = enode.toElement();
			QString type = e.tagName().toLower();
			QString ename = e.attribute(QLatin1String( "name" ));
			bool hasEntry = _w->hasEntry(ename);
			if (hasEntry && mp == Prompt) {
				KBetterThanKDialogBase *bd;
				bd = new KBetterThanKDialogBase(this);
				bd->setLabel(i18n("Folder '<b>%1</b>' already contains an entry '<b>%2</b>'.  Do you wish to replace it?", Qt::escape(fname), Qt::escape(ename)));
				mp = (MergePlan)bd->exec();
				delete bd;
				bool ok = false;
				if (mp == Always || mp == Yes) {
					ok = true;
				}
				if (mp == Yes || mp == No) { // reset mp
					mp = Prompt;
				}
				if (!ok) {
					enode = enode.nextSibling();
					continue;
				}
			} else if (hasEntry && mp == Never) {
				enode = enode.nextSibling();
				continue;
			}

			if (type == QLatin1String( "password" )) {
				_w->writePassword(ename, e.text());
			} else if (type == QLatin1String( "stream" )) {
				_w->writeEntry(ename, KCodecs::base64Decode(e.text().toLatin1()));
			} else if (type == QLatin1String( "map" )) {
				QMap<QString,QString> map;
				QDomNode mapNode = e.firstChild();
				while (!mapNode.isNull()) {
					QDomElement mape = mapNode.toElement();
					if (mape.tagName().toLower() == QLatin1String( "mapentry" )) {
						map[mape.attribute(QLatin1String( "name" ))] = mape.text();
					}
					mapNode = mapNode.nextSibling();
				}
				_w->writeMap(ename, map);
			}
			enode = enode.nextSibling();
		}
		n = n.nextSibling();
	}

	KIO::NetAccess::removeTempFile(tmpFile);
	updateFolderList(true);
	restoreEntry();
}


void KWalletEditor::exportXML() {
	KTemporaryFile tf;
	tf.open();
	QXmlStreamWriter xml(&tf);
	xml.setAutoFormatting(true);
	xml.writeStartDocument();
	const QStringList fl = _w->folderList();

	xml.writeStartElement(QLatin1String( "wallet" ));
	xml.writeAttribute(QLatin1String( "name" ), _walletName);
	for (QStringList::const_iterator i = fl.constBegin(); i != fl.constEnd(); ++i) {
		xml.writeStartElement(QLatin1String( "folder" ));
		xml.writeAttribute(QLatin1String( "name" ), *i);
		_w->setFolder(*i);
		QStringList entries = _w->entryList();
		for (QStringList::const_iterator j = entries.constBegin(); j != entries.constEnd(); ++j) {
			switch (_w->entryType(*j)) {
				case KWallet::Wallet::Password:
					{
						QString pass;
						if (_w->readPassword(*j, pass) == 0) {
							xml.writeStartElement(QLatin1String( "password" ));
							xml.writeAttribute(QLatin1String( "name" ), *j);
							xml.writeCharacters(pass);
							xml.writeEndElement();
						}
						break;
					}
				case KWallet::Wallet::Stream:
					{
						QByteArray ba;
						if (_w->readEntry(*j, ba) == 0) {
							xml.writeStartElement(QLatin1String( "stream" ));
							xml.writeAttribute(QLatin1String( "name" ), *j);
							xml.writeCharacters(QLatin1String( KCodecs::base64Encode(ba) ));
							xml.writeEndElement();
						}
						break;
					}
				case KWallet::Wallet::Map:
					{
						QMap<QString,QString> map;
						if (_w->readMap(*j, map) == 0) {
							xml.writeStartElement(QLatin1String( "map" ));
							xml.writeAttribute(QLatin1String( "name" ), *j);
							for (QMap<QString,QString>::ConstIterator k = map.constBegin(); k != map.constEnd(); ++k) {
								xml.writeStartElement(QLatin1String( "mapentry" ));
								xml.writeAttribute(QLatin1String( "name" ), k.key());
								xml.writeCharacters(k.value());
								xml.writeEndElement();
							}
							xml.writeEndElement();
						}
						break;
					}
				case KWallet::Wallet::Unknown:
				default:
					break;
			}
		}
		xml.writeEndElement();
	}

	xml.writeEndElement();
	xml.writeEndDocument();
	tf.flush();

	KUrl url = KFileDialog::getSaveUrl(KUrl(), QLatin1String( "*.xml" ), this);

	if (!url.isEmpty()) {
		KIO::NetAccess::dircopy(KUrl::fromPath(tf.fileName()), url, this);
	}
}


void KWalletEditor::setNewWallet(bool x) {
	_newWallet = x;
}


void KWalletEditor::saveAs() {
	KUrl url = KFileDialog::getSaveUrl(KUrl(), QLatin1String( "*.kwl" ), this);
	if (!url.isEmpty()) {
		// Sync() kwalletd
		if (_nonLocal) {
			KIO::NetAccess::dircopy(_walletName, url, this);
		} else {
			QString path = KGlobal::dirs()->saveLocation("kwallet") + QLatin1Char( '/' ) + _walletName + QLatin1String( ".kwl" );
			KIO::NetAccess::dircopy(KUrl::fromPath(path), url, this);
		}
	}
}


#include "kwalleteditor.moc"

