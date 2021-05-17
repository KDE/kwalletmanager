/*
    SPDX-FileCopyrightText: 2003-2005 George Staikos <staikos@kde.org>
    SPDX-FileCopyrightText: 2005 Isaac Clerencia <isaac@warp.es>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwalleteditor.h"
#include "kbetterthankdialogbase.h"
#include "kwmapeditor.h"
#include "allyourbase.h"

#include <QAction>
#include <QDialog>
#include <QInputDialog>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KActionCollection>
#include <KCodecs>
#include <KMessageBox>
#include <QMenu>
#include <KSqueezedTextLabel>
#include <KStandardAction>
#include <KConfigGroup>

#include <QTemporaryFile>
#include <KXMLGUIFactory>
#include <KSharedConfig>

#include <KToolBar>
#include <KLocalizedString>
#include <QIcon>
#include <KTreeWidgetSearchLine>

#include <QCheckBox>
#include <QClipboard>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QFileDialog>
#include <QList>
#include <QPushButton>
#include <QStack>
#include <QSet>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QXmlStreamWriter>

#include <assert.h>
#include <stdlib.h>

QAction *KWalletEditor::_newFolderAction = nullptr;
QAction *KWalletEditor::_deleteFolderAction = nullptr;
QAction *KWalletEditor::_exportAction = nullptr;
QAction *KWalletEditor::_mergeAction = nullptr;
QAction *KWalletEditor::_importAction = nullptr;
QAction *KWalletEditor::_newEntryAction = nullptr;
QAction *KWalletEditor::_renameEntryAction = nullptr;
QAction *KWalletEditor::_deleteEntryAction = nullptr;
QAction *KWalletEditor::_copyPassAction = nullptr;
QAction *KWalletEditor::_alwaysShowContentsAction = nullptr;
QAction *KWalletEditor::_alwaysHideContentsAction = nullptr;

RegisterCreateActionsMethod KWalletEditor::_registerCreateActionMethod(&KWalletEditor::createActions);

KWalletEditor::KWalletEditor(QWidget *parent, const QString &name)
    : QWidget(parent), _displayedItem(nullptr), _actionCollection(nullptr), _alwaysShowContents(false)
{
    setupUi(this);
    setObjectName(name);
    _newWallet = false;
    _splitter->setStretchFactor(0, 1);
    _splitter->setStretchFactor(1, 2);
    _contextMenu = new QMenu(this);

    _undoChanges->setIcon(QIcon::fromTheme(QStringLiteral("edit-undo")));
    _saveChanges->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
    _hasUnsavedChanges = false;

    auto box = new QVBoxLayout(_entryListFrame);
    box->setContentsMargins(0, 0, 0, 0);
    _entryList = new KWalletEntryList(_entryListFrame, QStringLiteral("Wallet Entry List"));
    _entryList->setContextMenuPolicy(Qt::CustomContextMenu);
    _searchLine = new KTreeWidgetSearchLine(_entryListFrame, _entryList);
    _searchLine->setPlaceholderText(i18n("Search"));
    connect(_searchLine, &KTreeWidgetSearchLine::textChanged, this, &KWalletEditor::onSearchTextChanged);
    box->addWidget(_searchLine);
    box->addWidget(_entryList);

    _entryStack->setEnabled(true);

    box = new QVBoxLayout(_entryStack->widget(2));
    box->setContentsMargins(0, 0, 0, 0);
    _mapEditorShowHide = new QCheckBox(i18n("&Show values"), _entryStack->widget(2));
    connect(_mapEditorShowHide, &QCheckBox::toggled, this, &KWalletEditor::showHideMapEditorValue);
    _mapEditor = new KWMapEditor(_currentMap, _entryStack->widget(2));
    box->addWidget(_mapEditorShowHide);
    box->addWidget(_mapEditor);

    // load splitter size
    KConfigGroup cg(KSharedConfig::openConfig(), "WalletEditor");
    QList<int> splitterSize = cg.readEntry("SplitterSize", QList<int>());
    if (splitterSize.size() != 2) {
        splitterSize.clear();
        splitterSize.append(_splitter->width() / 2);
        splitterSize.append(_splitter->width() / 2);
    }
    _splitter->setSizes(splitterSize);
    _alwaysShowContents = cg.readEntry("AlwaysShowContents", false);

    _searchLine->setFocus();

    connect(_entryList, &KWalletEntryList::currentItemChanged, this, &KWalletEditor::entrySelectionChanged);
    connect(_entryList, &KWalletEntryList::customContextMenuRequested, this, &KWalletEditor::listContextMenuRequested);
    connect(_entryList, &KWalletEntryList::itemChanged, this, &KWalletEditor::listItemChanged);

    connect(_passwordValue, &QTextEdit::textChanged, this, &KWalletEditor::entryEditted);
    connect(_mapEditor, &KWMapEditor::dirty, this, &KWalletEditor::entryEditted);

    connect(_undoChanges, &QPushButton::clicked, this, &KWalletEditor::restoreEntry);
    connect(_saveChanges, &QPushButton::clicked, this, &KWalletEditor::saveEntry);

    connect(_showContents, &QToolButton::clicked, this, &KWalletEditor::showPasswordContents);
    connect(_hideContents, &QToolButton::clicked, this, &KWalletEditor::hidePasswordContents);

    _passwordValue->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    _binaryViewShow->setChecked(_alwaysShowContents);
//    createActions();
    // TODO: remove kwalleteditor.rc file
}

KWalletEditor::~KWalletEditor()
{
    Q_EMIT enableFolderActions(false);
    Q_EMIT enableWalletActions(false);
    Q_EMIT enableContextFolderActions(false);
    // save splitter size
    KConfigGroup cg(KSharedConfig::openConfig(), "WalletEditor");
    cg.writeEntry("SplitterSize", _splitter->sizes());
    cg.writeEntry("AlwaysShowContents", _alwaysShowContents);
    cg.sync();

    delete _w;
    _w = nullptr;
    if (_nonLocal) {
        KWallet::Wallet::closeWallet(_walletName, true);
    }
    delete _contextMenu;
    _contextMenu = nullptr;
}

void KWalletEditor::setWallet(KWallet::Wallet *wallet, bool isPath)
{
    Q_ASSERT(wallet != nullptr);
    _walletName = wallet->walletName();
    _nonLocal = isPath;

    _w = wallet;
    _entryList->setWallet(_w);
    connect(_w, &KWallet::Wallet::walletOpened, this, &KWalletEditor::walletOpened);
    connect(_w, &KWallet::Wallet::walletClosed, this, &KWalletEditor::walletClosed);
    connect(_w, &KWallet::Wallet::folderUpdated, this, &KWalletEditor::updateEntries);
    connect(_w, SIGNAL(folderListUpdated()), this, SLOT(updateFolderList()));
    updateFolderList();

    Q_EMIT enableFolderActions(true);
    Q_EMIT enableWalletActions(true);
    Q_EMIT enableContextFolderActions(true);

    _mapEditorShowHide->setChecked(false);
    showHideMapEditorValue(false);

    setFocus();
    _searchLine->setFocus();
}

bool KWalletEditor::isOpen() const
{
    return _w != nullptr;
}

KActionCollection *KWalletEditor::actionCollection()
{
    if (!_actionCollection) {
        _actionCollection = new KActionCollection(this);
    }
    return _actionCollection;
}

void KWalletEditor::createActions(KActionCollection *actionCollection)
{
    _newFolderAction = actionCollection->addAction(QStringLiteral("create_folder"));
    _newFolderAction->setText(i18n("&New Folder..."));
    _newFolderAction->setIcon(QIcon::fromTheme(QStringLiteral("folder-new")));

    _deleteFolderAction = actionCollection->addAction(QStringLiteral("delete_folder"));
    _deleteFolderAction->setText(i18n("&Delete Folder"));

    _mergeAction = actionCollection->addAction(QStringLiteral("wallet_merge"));
    _mergeAction->setText(i18n("&Import a wallet..."));

    _importAction = actionCollection->addAction(QStringLiteral("wallet_import"));
    _importAction->setText(i18n("&Import XML..."));

    _exportAction = actionCollection->addAction(QStringLiteral("wallet_export"));
    _exportAction->setText(i18n("&Export as XML..."));

    _copyPassAction = actionCollection->addAction(QStringLiteral("copy_action"));
    _copyPassAction->setText(i18n("&Copy"));
    actionCollection->setDefaultShortcut(_copyPassAction, Qt::Key_C | Qt::CTRL);
    _copyPassAction->setEnabled(false);

    _newEntryAction = actionCollection->addAction(QStringLiteral("new_entry"));
    _newEntryAction->setText(i18n("&New..."));
    actionCollection->setDefaultShortcut(_newEntryAction, Qt::Key_Insert);
    _newEntryAction->setEnabled(false);

    _renameEntryAction = actionCollection->addAction(QStringLiteral("rename_entry"));
    _renameEntryAction->setText(i18n("&Rename"));
    actionCollection->setDefaultShortcut(_renameEntryAction, Qt::Key_F2);
    _renameEntryAction->setEnabled(false);

    _deleteEntryAction = actionCollection->addAction(QStringLiteral("delete_entry"));
    _deleteEntryAction->setText(i18n("&Delete"));
    actionCollection->setDefaultShortcut(_deleteEntryAction, Qt::Key_Delete);
    _deleteEntryAction->setEnabled(false);

    _alwaysShowContentsAction = actionCollection->addAction(QStringLiteral("always_show_contents"));
    _alwaysShowContentsAction->setText(i18n("Always show contents"));
    _alwaysShowContentsAction->setCheckable(true);

    _alwaysHideContentsAction = actionCollection->addAction(QStringLiteral("always_hide_contents"));
    _alwaysHideContentsAction->setText(i18n("Always hide contents"));
    _alwaysHideContentsAction->setCheckable(true);
}

void KWalletEditor::connectActions()
{
    connect(_newFolderAction, &QAction::triggered, this, &KWalletEditor::createFolder);
    connect(this, &KWalletEditor::enableFolderActions, _newFolderAction, &QAction::setEnabled);

    connect(_deleteFolderAction, &QAction::triggered, this, &KWalletEditor::deleteFolder);
    connect(this, &KWalletEditor::enableContextFolderActions, _deleteFolderAction, &QAction::setEnabled);
    connect(this, &KWalletEditor::enableFolderActions, _deleteFolderAction, &QAction::setEnabled);

    connect(_mergeAction, &QAction::triggered, this, &KWalletEditor::importWallet);
    connect(this, &KWalletEditor::enableWalletActions, _mergeAction, &QAction::setEnabled);

    connect(_importAction, &QAction::triggered, this, &KWalletEditor::importXML);
    connect(this, &KWalletEditor::enableWalletActions, _importAction, &QAction::setEnabled);

    connect(_exportAction, &QAction::triggered, this, &KWalletEditor::exportXML);
    connect(this, &KWalletEditor::enableWalletActions, _exportAction, &QAction::setEnabled);

    connect(_newEntryAction, &QAction::triggered, this, &KWalletEditor::newEntry);

    connect(_renameEntryAction, &QAction::triggered, this, &KWalletEditor::renameEntry);

    connect(_deleteEntryAction, &QAction::triggered, this, &KWalletEditor::deleteEntry);

    connect(_copyPassAction, &QAction::triggered, this, &KWalletEditor::copyPassword);
    connect(this, &KWalletEditor::enableWalletActions, _copyPassAction, &QAction::setEnabled);

    _showContents->addAction(_alwaysShowContentsAction);
    _alwaysShowContentsAction->setChecked(_alwaysShowContents);
    connect(_alwaysShowContentsAction, &QAction::triggered, this, &KWalletEditor::onAlwaysShowContents);

    _hideContents->addAction(_alwaysHideContentsAction);
    _alwaysHideContentsAction->setChecked(!_alwaysShowContents);
    connect(_alwaysHideContentsAction, &QAction::triggered, this, &KWalletEditor::onAlwaysHideContents);
}

void KWalletEditor::disconnectActions()
{
    disconnect(_newFolderAction, &QAction::triggered, this, &KWalletEditor::createFolder);
    disconnect(this, &KWalletEditor::enableFolderActions, _newFolderAction, &QAction::setEnabled);

    disconnect(_deleteFolderAction, &QAction::triggered, this, &KWalletEditor::deleteFolder);
    disconnect(this, &KWalletEditor::enableContextFolderActions, _deleteFolderAction, &QAction::setEnabled);
    disconnect(this, &KWalletEditor::enableFolderActions, _deleteFolderAction, &QAction::setEnabled);

    disconnect(_mergeAction, &QAction::triggered, this, &KWalletEditor::importWallet);
    disconnect(this, &KWalletEditor::enableWalletActions, _mergeAction, &QAction::setEnabled);

    disconnect(_importAction, &QAction::triggered, this, &KWalletEditor::importXML);
    disconnect(this, &KWalletEditor::enableWalletActions, _importAction, &QAction::setEnabled);

    disconnect(_exportAction, &QAction::triggered, this, &KWalletEditor::exportXML);
    disconnect(this, &KWalletEditor::enableWalletActions, _exportAction, &QAction::setEnabled);

    disconnect(_newEntryAction, &QAction::triggered, this, &KWalletEditor::newEntry);

    disconnect(_renameEntryAction, &QAction::triggered, this, &KWalletEditor::renameEntry);

    disconnect(_deleteEntryAction, &QAction::triggered, this, &KWalletEditor::deleteEntry);

    disconnect(_copyPassAction, &QAction::triggered, this, &KWalletEditor::copyPassword);
    disconnect(this, &KWalletEditor::enableWalletActions, _copyPassAction, &QAction::setEnabled);

    disconnect(_alwaysShowContentsAction, &QAction::triggered, this, &KWalletEditor::onAlwaysShowContents);
    disconnect(_alwaysHideContentsAction, &QAction::triggered, this, &KWalletEditor::onAlwaysHideContents);
}

void KWalletEditor::walletClosed()
{
    _w = nullptr;
    setEnabled(false);
    Q_EMIT enableWalletActions(false);
    Q_EMIT enableFolderActions(false);
}

void KWalletEditor::updateFolderList(bool checkEntries)
{
    const QStringList fl = _w->folderList();
    QStack<QTreeWidgetItem *> trash;

    for (int i = 0; i < _entryList->topLevelItemCount(); ++i) {
        auto fi = dynamic_cast<KWalletFolderItem *>(_entryList->topLevelItem(i));
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
        auto item = new KWalletFolderItem(_w, _entryList, *i, entries.count());

        auto pi = new KWalletContainerItem(item, i18n("Passwords"), KWallet::Wallet::Password);
        auto mi = new KWalletContainerItem(item, i18n("Maps"), KWallet::Wallet::Map);
        auto bi = new KWalletContainerItem(item, i18n("Binary Data"), KWallet::Wallet::Stream);
        auto ui = new KWalletContainerItem(item, i18n("Unknown"), KWallet::Wallet::Unknown);

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
        _entryTitle->clear();
        _iconTitle->clear();
    }
}

void KWalletEditor::deleteFolder()
{
    if (_w) {
        QTreeWidgetItem *i = _entryList->currentItem();
        if (i) {
            auto fi = dynamic_cast<KWalletFolderItem *>(i);
            if (!fi) {
                return;
            }

            int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the folder '%1' from the wallet?", fi->name()), QString(), KStandardGuiItem::del());
            if (rc == KMessageBox::Continue) {
                bool rc = _w->removeFolder(fi->name());
                if (!rc) {
                    KMessageBox::sorry(this, i18n("Error deleting folder."));
                    return;
                }
                _currentFolder.clear();
                _entryTitle->clear();
                _iconTitle->clear();
                updateFolderList();
            }
        }
    }
}

void KWalletEditor::createFolder()
{
    if (_w) {
        QString n;
        bool ok;

        do {
            n = QInputDialog::getText(this, i18n("New Folder"),
                                      i18n("Please choose a name for the new folder:"),
                                      QLineEdit::Normal, QString(),
                                      &ok);

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

void KWalletEditor::saveEntry()
{
    int rc = 1;
    QTreeWidgetItem *item = _displayedItem; //  _entryList->currentItem();
    _saveChanges->setEnabled(false);
    _undoChanges->setEnabled(false);
    _hasUnsavedChanges = false;
    if (item && _w && item->parent()) {
        auto ci = dynamic_cast<KWalletContainerItem *>(item->parent());
        if (ci) {
            if (ci->entryType() == KWallet::Wallet::Password) {
                rc = _w->writePassword(item->text(0), _passwordValue->toPlainText());
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

void KWalletEditor::restoreEntry()
{
    entrySelectionChanged(_entryList->currentItem());
    _hasUnsavedChanges = false;
}

void KWalletEditor::entryEditted()
{
    _saveChanges->setEnabled(true);
    _undoChanges->setEnabled(true);
    _hasUnsavedChanges = true;
}

void KWalletEditor::entrySelectionChanged(QTreeWidgetItem *item)
{
    // do not forget to save changes
    if (_saveChanges->isEnabled() && _displayedItem && (_displayedItem != item)) {
        if (KMessageBox::Yes ==  KMessageBox::questionYesNo(this,
                i18n("The contents of the current item has changed.\nDo you want to save changes?"))) {
            saveEntry();
        } else {
            _saveChanges->setEnabled(false);
            _undoChanges->setEnabled(false);
            _hasUnsavedChanges = false;
        }
    }

    KWalletContainerItem *ci = nullptr;
    KWalletFolderItem *fi = nullptr;

    // clear the context menu
    _contextMenu->clear();
    _contextMenu->setEnabled(true);
    // disable the entry actions (reenable them on adding)
    _newEntryAction->setEnabled(false);
    _renameEntryAction->setEnabled(false);
    _deleteEntryAction->setEnabled(false);

    if (item) {
        // set the context menu's title
        _contextMenu->addSection(_contextMenu->fontMetrics().elidedText(
                                   item->text(0), Qt::ElideMiddle, 200));

        // TODO rtti
        switch (item->type()) {
        case KWalletEntryItemClass:
            ci = dynamic_cast<KWalletContainerItem *>(item->parent());
            if (!ci) {
                return;
            }
            fi = dynamic_cast<KWalletFolderItem *>(ci->parent());
            if (!fi) {
                return;
            }
            _w->setFolder(fi->name());
            _deleteFolderAction->setEnabled(false);

            // add standard menu items
            _contextMenu->addAction(_newEntryAction);
            _contextMenu->addAction(_renameEntryAction);
            _contextMenu->addAction(_deleteEntryAction);
            _newEntryAction->setEnabled(true);
            _renameEntryAction->setEnabled(true);
            _deleteEntryAction->setEnabled(true);

            if (ci->entryType() == KWallet::Wallet::Password) {
                QString pass;
                if (_w->readPassword(item->text(0), pass) == 0) {
                    _entryStack->setCurrentIndex(4);
                    _entryName->setText(i18n("Password: %1",
                                             item->text(0)));
                    _passwordValue->setText(pass);
                    _saveChanges->setEnabled(false);
                    _undoChanges->setEnabled(false);
                    _hasUnsavedChanges = false;
                }
                // add a context-menu action for copying passwords
                _contextMenu->addSeparator();
                _contextMenu->addAction(_copyPassAction);
                if (_alwaysShowContents) {
                    QTimer::singleShot(0, this, &KWalletEditor::showPasswordContents);
                }
            } else if (ci->entryType() == KWallet::Wallet::Map) {
                _entryStack->setCurrentIndex(2);
                if (_w->readMap(item->text(0), _currentMap) == 0) {
                    _mapEditor->reload();
                    _entryName->setText(i18n("Name-Value Map: %1", item->text(0)));
                    _saveChanges->setEnabled(false);
                    _undoChanges->setEnabled(false);
                    _hasUnsavedChanges = false;
                    showHideMapEditorValue(_mapEditorShowHide->isChecked());
                }
            } else if (ci->entryType() == KWallet::Wallet::Stream) {
                _entryStack->setCurrentIndex(3);
                QByteArray ba;
                if (_w->readEntry(item->text(0), ba) == 0) {
                    _entryName->setText(i18n("Binary Data: %1",
                                             item->text(0)));
                    _saveChanges->setEnabled(false);
                    _undoChanges->setEnabled(false);
                    _hasUnsavedChanges = false;
                    _binaryView->setData(ba);
                }
            }
            break;

        case KWalletContainerItemClass:
            ci = dynamic_cast<KWalletContainerItem *>(item);
            if (!ci) {
                return;
            }
            if (ci->entryType() == KWallet::Wallet::Unknown) {
                // disable context menu on unknown items
                _contextMenu->setEnabled(false);
            } else {
                // add the context menu action
                _contextMenu->addAction(_newEntryAction);
                _newEntryAction->setEnabled(true);
            }

            fi = dynamic_cast<KWalletFolderItem *>(item->parent());
            if (!fi) {
                return;
            }
            _w->setFolder(fi->name());
            _deleteFolderAction->setEnabled(false);
            _entryName->clear();
            _entryStack->setCurrentIndex(0);
            break;

        case KWalletFolderItemClass:
            // add the context menu actions
            _contextMenu->addAction(_newFolderAction);
            _contextMenu->addAction(_deleteFolderAction);

            fi = dynamic_cast<KWalletFolderItem *>(item);
            if (!fi) {
                return;
            }
            _w->setFolder(fi->name());
            _deleteFolderAction->setEnabled(true);
            _entryName->clear();
            _entryStack->setCurrentIndex(0);
            break;

        default:
            // all items but Unknown entries return have their
            // rtti set. Unknown items can only be deleted.
            _contextMenu->addAction(_deleteEntryAction);
            _deleteEntryAction->setEnabled(true);
            break;
        }
    } else {
        // no item selected. add the "new folder" action to the context menu
        _contextMenu->addAction(_newFolderAction);
    }

    if (fi) {
        _currentFolder = fi->name();
        _entryTitle->setText(QStringLiteral("<font size=\"+1\">%1</font>").arg(fi->text(0)));
        _iconTitle->setPixmap(fi->getFolderIcon().pixmap(style()->pixelMetric(QStyle::PixelMetric::PM_ToolBarIconSize)));
    }

    _displayedItem = item;
}

void KWalletEditor::updateEntries(const QString &folder)
{
    QStack<QTreeWidgetItem *> trash;

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
                entrySelectionChanged(nullptr);
            }
            trash.push(twi);
        }
    }

    for (int i = 0; i < mi->childCount(); ++i) {
        QTreeWidgetItem *twi = mi->child(i);
        if (!entries.contains(twi->text(0))) {
            if (twi == _entryList->currentItem()) {
                entrySelectionChanged(nullptr);
            }
            trash.push(twi);
        }
    }

    for (int i = 0; i < bi->childCount(); ++i) {
        QTreeWidgetItem *twi = bi->child(i);
        if (!entries.contains(twi->text(0))) {
            if (twi == _entryList->currentItem()) {
                entrySelectionChanged(nullptr);
            }
            trash.push(twi);
        }
    }

    for (int i = 0; i < ui->childCount(); ++i) {
        QTreeWidgetItem *twi = ui->child(i);
        if (!entries.contains(twi->text(0))) {
            if (twi == _entryList->currentItem()) {
                entrySelectionChanged(nullptr);
            }
            trash.push(twi);
        }
    }

    qDeleteAll(trash);
    trash.clear();

    // Add new entries
    for (QStringList::const_iterator i = entries.begin(), end = entries.end(); i != end; ++i) {
        if (fi->contains(*i)) {
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
        _entryTitle->setText(QStringLiteral("<font size=\"+1\">%1</font>").arg(fi->text(0)));
    }
    if (!_entryList->currentItem()) {
        _entryName->clear();
        _entryStack->setCurrentIndex(0);
    }
}

void KWalletEditor::listContextMenuRequested(const QPoint &pos)
{
    if (!_contextMenu->isEnabled()) {
        return;
    }

    _contextMenu->popup(_entryList->mapToGlobal(pos));
}

void KWalletEditor::copyPassword()
{
    QTreeWidgetItem *item = _entryList->currentItem();
    if (_w && item) {
        QString pass;
        if (_w->readPassword(item->text(0), pass) == 0) {
            QApplication::clipboard()->setText(pass);
        }
    }
}

void KWalletEditor::newEntry()
{
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
        n = QInputDialog::getText(this, i18n("New Entry"),
                                  i18n("Please choose a name for the new entry:"),
                                  QLineEdit::Normal, QString(),
                                  &ok);

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

        auto fi = dynamic_cast<KWalletFolderItem *>(p->parent());
        if (!fi) {
            KMessageBox::error(this, i18n("An unexpected error occurred trying to add the new entry"));
            return;
        }
        _w->setFolder(fi->name());

        auto ni = new KWalletEntryItem(_w, p, n);

        auto ci = dynamic_cast<KWalletContainerItem *>(p);
        if (!ci) {
            KMessageBox::error(this, i18n("An unexpected error occurred trying to add the new entry"));
            delete ni;
            return;
        }
        if (ci->entryType() == KWallet::Wallet::Password) {
            _w->writePassword(n, QString());
        } else if (ci->entryType() == KWallet::Wallet::Map) {
            _w->writeMap(n, QMap<QString, QString>());
        } else if (ci->entryType() == KWallet::Wallet::Stream) {
            _w->writeEntry(n, QByteArray());
        } else {
            abort();
        }

        _entryList->setCurrentItem(ni);
        _entryList->scrollToItem(ni);

        fi->refresh();
        _entryTitle->setText(QStringLiteral("<font size=\"+1\">%1</font>").arg(fi->text(0)));
    }
}

void KWalletEditor::renameEntry()
{
    QTreeWidgetItem *item = _entryList->currentItem();
    if (_w && item) {
        _entryList->editItem(item, 0);
    }
}

// Only supports renaming of KWalletEntryItem derived classes.
void KWalletEditor::listItemChanged(QTreeWidgetItem *item, int column)
{
    if (item && column == 0) {
        auto i = dynamic_cast<KWalletEntryItem *>(item);
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
            auto ci = dynamic_cast<KWalletContainerItem *>(item->parent());
            if (!ci) {
                KMessageBox::error(this, i18n("An unexpected error occurred trying to rename the entry"));
                return;
            }
            if (ci->entryType() == KWallet::Wallet::Password) {
                _entryName->setText(i18n("Password: %1", item->text(0)));
            } else if (ci->entryType() == KWallet::Wallet::Map) {
                _entryName->setText(i18n("Name-Value Map: %1", item->text(0)));
            } else if (ci->entryType() == KWallet::Wallet::Stream) {
                _entryName->setText(i18n("Binary Data: %1", item->text(0)));
            }
        } else {
            i->restoreName();
        }
    }
}

void KWalletEditor::deleteEntry()
{
    QTreeWidgetItem *item = _entryList->currentItem();
    if (_w && item) {
        int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the item '%1'?", item->text(0)), QString(), KStandardGuiItem::del());
        if (rc == KMessageBox::Continue) {
            auto fi = dynamic_cast<KWalletFolderItem *>(item->parent()->parent());
            if (!fi) {
                KMessageBox::error(this, i18n("An unexpected error occurred trying to delete the entry"));
                return;
            }
            _displayedItem = nullptr;
            _w->removeEntry(item->text(0));
            delete item;
            entrySelectionChanged(_entryList->currentItem());
            fi->refresh();
            _entryTitle->setText(QStringLiteral("<font size=\"+1\">%1</font>").arg(fi->text(0)));
        }
    }
}

void KWalletEditor::changePassword()
{
    KWallet::Wallet::changePassword(_walletName, effectiveWinId());
}

void KWalletEditor::walletOpened(bool success)
{
    if (success) {
        Q_EMIT enableFolderActions(true);
        Q_EMIT enableContextFolderActions(false);
        Q_EMIT enableWalletActions(true);
        updateFolderList();
        _entryList->setWallet(_w);
    } else {
        if (!_newWallet) {
            KMessageBox::sorry(this, i18n("Unable to open the requested wallet."));
        }
    }
}

void KWalletEditor::hidePasswordContents()
{
    _entryStack->setCurrentIndex(4);
}

void KWalletEditor::showPasswordContents()
{
    _entryStack->setCurrentIndex(1);
}

void KWalletEditor::showHideMapEditorValue(bool show)
{
    if (show) {
        _mapEditor->showColumn(2);
    } else {
        _mapEditor->hideColumn(2);
    }
}

enum MergePlan { Prompt = 0, Always = 1, Never = 2, Yes = 3, No = 4 };

void KWalletEditor::importWallet()
{
    QUrl url = QFileDialog::getOpenFileUrl(this, QString(), QUrl(), QStringLiteral("*.kwl"));

    if (url.isEmpty()) {
        return;
    }

    QTemporaryFile tmpFile;
    if (!tmpFile.open()) {
        KMessageBox::sorry(this, i18n("Unable to create temporary file for downloading '<b>%1</b>'.", url.toDisplayString()));
        return;
    }

    KIO::StoredTransferJob *job = KIO::storedGet(url);
    KJobWidgets::setWindow(job, this);
    if (!job->exec()) {
        KMessageBox::sorry(this, i18n("Unable to access wallet '<b>%1</b>'.", url.toDisplayString()));
        return;
    }
    tmpFile.write(job->data());
    tmpFile.flush();

    KWallet::Wallet *w = KWallet::Wallet::openWallet(tmpFile.fileName(), effectiveWinId(), KWallet::Wallet::Path);
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

            bool readMap = false;
            QMap<QString, QMap<QString, QString>> map = w->mapList(&readMap);
            QSet<QString> mergedkeys; // prevents re-merging already merged entries.
            if (readMap) {
                QMap<QString, QMap<QString, QString> >::ConstIterator me;
                for (me = map.constBegin(); me != map.constEnd(); ++me) {
                    bool hasEntry = _w->hasEntry(me.key());
                    if (hasEntry && mp == Prompt) {
                        KBetterThanKDialogBase *bd;
                        bd = new KBetterThanKDialogBase(this);
                        bd->setLabel(i18n("Folder '<b>%1</b>' already contains an entry '<b>%2</b>'.  Do you wish to replace it?", f->toHtmlEscaped(), me.key().toHtmlEscaped()));
                        mp = static_cast<MergePlan>(bd->exec());
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

            bool readPassList = false;
            QMap<QString, QString> pwd = w->passwordList(&readPassList);
            if (readPassList) {
                QMap<QString, QString>::ConstIterator pe;
                for (pe = pwd.constBegin(); pe != pwd.constEnd(); ++pe) {
                    bool hasEntry = _w->hasEntry(pe.key());
                    if (hasEntry && mp == Prompt) {
                        auto bd = new KBetterThanKDialogBase(this);
                        bd->setLabel(i18n("Folder '<b>%1</b>' already contains an entry '<b>%2</b>'.  Do you wish to replace it?", f->toHtmlEscaped(), pe.key().toHtmlEscaped()));
                        mp = static_cast<MergePlan>(bd->exec());
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

            bool readEntries = false;
            QMap<QString, QByteArray> ent = w->entriesList(&readEntries);
            if (readEntries) {
                QMap<QString, QByteArray>::ConstIterator ee;
                for (ee = ent.constBegin(); ee != ent.constEnd(); ++ee) {
                    // prevent re-merging already merged entries.
                    if (mergedkeys.contains(ee.key())) {
                        continue;
                    }
                    bool hasEntry = _w->hasEntry(ee.key());
                    if (hasEntry && mp == Prompt) {
                        auto bd = new KBetterThanKDialogBase(this);
                        bd->setLabel(i18n("Folder '<b>%1</b>' already contains an entry '<b>%2</b>'.  Do you wish to replace it?", f->toHtmlEscaped(), ee.key().toHtmlEscaped()));
                        mp = static_cast<MergePlan>(bd->exec());
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

    updateFolderList(true);
    restoreEntry();
}

void KWalletEditor::importXML()
{
    const QUrl url = QFileDialog::getOpenFileUrl(this, QString(), QUrl(), QStringLiteral("*.xml"));

    if (url.isEmpty()) {
        return;
    }

    KIO::StoredTransferJob *job = KIO::storedGet(url);
    KJobWidgets::setWindow(job, this);
    if (!job->exec()) {
        KMessageBox::sorry(this, i18n("Unable to access XML file '<b>%1</b>'.", url.toDisplayString()));
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(job->data())) {
        KMessageBox::sorry(this, i18n("Error reading XML file '<b>%1</b>' for input.", url.toDisplayString()));
        return;
    }

    QDomElement top = doc.documentElement();
    if (top.tagName().toLower() != QLatin1String("wallet")) {
        KMessageBox::sorry(this, i18n("Error: XML file does not contain a wallet."));
        return;
    }

    QDomNode n = top.firstChild();
    MergePlan mp = Prompt;
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (e.tagName().toLower() != QLatin1String("folder")) {
            n = n.nextSibling();
            continue;
        }

        QString fname = e.attribute(QStringLiteral("name"));
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
            QString ename = e.attribute(QStringLiteral("name"));
            bool hasEntry = _w->hasEntry(ename);
            if (hasEntry && mp == Prompt) {
                auto bd = new KBetterThanKDialogBase(this);
                bd->setLabel(i18n("Folder '<b>%1</b>' already contains an entry '<b>%2</b>'.  Do you wish to replace it?", fname.toHtmlEscaped(), ename.toHtmlEscaped()));
                mp = static_cast<MergePlan>(bd->exec());
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

            if (type == QLatin1String("password")) {
                _w->writePassword(ename, e.text());
            } else if (type == QLatin1String("stream")) {
                _w->writeEntry(ename, KCodecs::base64Decode(e.text().toLatin1()));
            } else if (type == QLatin1String("map")) {
                QMap<QString, QString> map;
                QDomNode mapNode = e.firstChild();
                while (!mapNode.isNull()) {
                    QDomElement mape = mapNode.toElement();
                    if (mape.tagName().toLower() == QLatin1String("mapentry")) {
                        map[mape.attribute(QStringLiteral("name"))] = mape.text();
                    }
                    mapNode = mapNode.nextSibling();
                }
                _w->writeMap(ename, map);
            }
            enode = enode.nextSibling();
        }
        n = n.nextSibling();
    }

    updateFolderList(true);
    restoreEntry();
}

void KWalletEditor::exportXML()
{
    QTemporaryFile tf;
    tf.open();
    QXmlStreamWriter xml(&tf);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    const QStringList fl = _w->folderList();

    xml.writeStartElement(QStringLiteral("wallet"));
    xml.writeAttribute(QStringLiteral("name"), _walletName);
    for (QStringList::const_iterator i = fl.constBegin(), flEnd(fl.constEnd()); i != flEnd; ++i) {
        xml.writeStartElement(QStringLiteral("folder"));
        xml.writeAttribute(QStringLiteral("name"), *i);
        _w->setFolder(*i);
        QStringList entries = _w->entryList();
        for (QStringList::const_iterator j = entries.constBegin(), entriesEnd(entries.constEnd()); j != entriesEnd; ++j) {
            switch (_w->entryType(*j)) {
            case KWallet::Wallet::Password: {
                QString pass;
                if (_w->readPassword(*j, pass) == 0) {
                    xml.writeStartElement(QStringLiteral("password"));
                    xml.writeAttribute(QStringLiteral("name"), *j);
                    xml.writeCharacters(pass);
                    xml.writeEndElement();
                }
                break;
            }
            case KWallet::Wallet::Stream: {
                QByteArray ba;
                if (_w->readEntry(*j, ba) == 0) {
                    xml.writeStartElement(QStringLiteral("stream"));
                    xml.writeAttribute(QStringLiteral("name"), *j);
                    xml.writeCharacters(QLatin1String(KCodecs::base64Encode(ba)));
                    xml.writeEndElement();
                }
                break;
            }
            case KWallet::Wallet::Map: {
                QMap<QString, QString> map;
                if (_w->readMap(*j, map) == 0) {
                    xml.writeStartElement(QStringLiteral("map"));
                    xml.writeAttribute(QStringLiteral("name"), *j);
                    for (QMap<QString, QString>::ConstIterator k = map.constBegin(), mapEnd(map.constEnd()); k != mapEnd; ++k) {
                        xml.writeStartElement(QStringLiteral("mapentry"));
                        xml.writeAttribute(QStringLiteral("name"), k.key());
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

    QUrl url = QFileDialog::getSaveFileUrl(this, QString(), QUrl(), QStringLiteral("*.xml"));

    if (url.isEmpty()) {
        return;
    }

    // #368314: QTemporaryFiles are open in ReadWrite mode, so the QIODevice's position needs to be rewind.
    tf.seek(0);

    KIO::StoredTransferJob *putJob = KIO::storedPut(&tf, url, -1);
    KJobWidgets::setWindow(putJob, this);
    if (!putJob->exec()) {
        KMessageBox::sorry(this, i18n("Unable to store to '<b>%1</b>'.", url.toDisplayString()));
    }
}

void KWalletEditor::setNewWallet(bool x)
{
    _newWallet = x;
}

void KWalletEditor::hideEvent(QHideEvent *)
{
    Q_EMIT enableContextFolderActions(false);
    Q_EMIT enableFolderActions(false);
    Q_EMIT enableWalletActions(false);
    disconnectActions();
}

void KWalletEditor::showEvent(QShowEvent *)
{
    connectActions();
    Q_EMIT enableContextFolderActions(true);
    Q_EMIT enableFolderActions(true);
    Q_EMIT enableWalletActions(true);
}

void KWalletEditor::onSearchTextChanged(const QString &text)
{
    static bool treeIsExpanded = false;
    if (text.isEmpty()) {
        if (treeIsExpanded) {
            _entryList->setCurrentItem(nullptr);
            // NOTE: the 300 ms here is a value >200 ms used internally by KTreeWidgetSearchLine
            // TODO: replace this timer with a connection to KTreeWidgetSearchLine::searchUpdated signal introduced with KF5
            QTimer::singleShot(300, _entryList, &KWalletEntryList::collapseAll);
            treeIsExpanded = false;
        }
    } else {
        if (!treeIsExpanded) {
            _entryList->expandAll();
            treeIsExpanded = true;
        }
        // NOTE: the 300 ms here is a value >200 ms used internally by KTreeWidgetSearchLine
        // TODO: replace this timer with a connection to KTreeWidgetSearchLine::searchUpdated signal introduced with KF5
        QTimer::singleShot(300, _entryList, &KWalletEntryList::selectFirstVisible);
    }
    // TODO: reduce timer count when KTreeWidgetSearchLine::searchUpdated signal will be there
    QTimer::singleShot(300, _entryList, &KWalletEntryList::refreshItemsCount);
}

void KWalletEditor::onAlwaysShowContents(bool checked)
{
    _alwaysShowContents = checked;
    _alwaysHideContentsAction->setChecked(!_alwaysShowContents);
    showPasswordContents();
}

void KWalletEditor::onAlwaysHideContents(bool checked)
{
    _alwaysShowContents = !checked;
    _alwaysShowContentsAction->setChecked(_alwaysShowContents);
    if (checked) {
        hidePasswordContents();
    }
}

bool KWalletEditor::hasUnsavedChanges() const
{
    return _hasUnsavedChanges;
}
