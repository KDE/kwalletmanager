/*
    SPDX-FileCopyrightText: 2003-2005 George Staikos <staikos@kde.org>
    SPDX-FileCopyrightText: 2005 Isaac Clerencia <isaac@warp.es>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWALLETEDITOR_H
#define KWALLETEDITOR_H

#include "registercreateactionmethod.h"
#include "ui_walletwidget.h"

#include <KWallet>

#include <QLabel>

class KActionCollection;
class QMenu;
class QTreeWidgetItem;
class QCheckBox;
class KWalletEntryList;
class KWMapEditor;
class KTreeWidgetSearchLine;

class KWalletEditor : public QWidget, public Ui::WalletWidget
{
    Q_OBJECT

public:
    explicit KWalletEditor(QWidget *parent, const QString &name = QString());
    ~KWalletEditor() override;

    void setWallet(KWallet::Wallet *wallet, bool isPath = false);
    bool isOpen() const;

    bool hasUnsavedChanges() const;
    void setNewWallet(bool newWallet);

protected:
    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;

public Q_SLOTS:
    void walletClosed();
    void createFolder();
    void deleteFolder();

private Q_SLOTS:
    void updateFolderList(bool checkEntries = false);
    void entrySelectionChanged(QTreeWidgetItem *item);
    void listItemChanged(QTreeWidgetItem *, int column);
    void listContextMenuRequested(const QPoint &pos);
    void updateEntries(const QString &folder);

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

    void exportXML();
    void importXML();
    void importWallet();

    void copyPassword();

    void onSearchTextChanged(const QString &);
    void onAlwaysShowContents(bool);
    void onAlwaysHideContents(bool);

Q_SIGNALS:
    void enableWalletActions(bool enable);
    void enableFolderActions(bool enable);
    void enableContextFolderActions(bool enable);

public:
    QString _walletName;

private:
    static void createActions(KActionCollection *);
    void connectActions();
    void disconnectActions();
    KActionCollection *actionCollection();

    bool _nonLocal;
    KWallet::Wallet *_w = nullptr;
    KWalletEntryList *_entryList = nullptr;
    static RegisterCreateActionsMethod _registerCreateActionMethod;
    static QAction *_newFolderAction, *_deleteFolderAction;
    static QAction *_exportAction, *_saveAsAction, *_mergeAction, *_importAction;
    static QAction *_newEntryAction, *_renameEntryAction, *_deleteEntryAction;
    static QAction *_copyPassAction;
    QString _currentFolder;
    QMap<QString, QString> _currentMap; // save memory by storing
    // only the most recent map.
    KWMapEditor *_mapEditor = nullptr;
    QCheckBox *_mapEditorShowHide = nullptr;
    bool _newWallet;
    QMenu *_contextMenu = nullptr;
    QTreeWidgetItem *_displayedItem = nullptr; // used to find old item when selection just changed
    KActionCollection *_actionCollection = nullptr;
    QMenu *_controlMenu = nullptr;
    QMenu *_walletSubmenu = nullptr;
    KTreeWidgetSearchLine *_searchLine = nullptr;
    static QAction *_alwaysShowContentsAction, *_alwaysHideContentsAction;
    bool _alwaysShowContents;
    bool _hasUnsavedChanges;
};

#endif
