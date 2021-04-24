/*
    SPDX-FileCopyrightText: 2003-2005 George Staikos <staikos@kde.org>
    SPDX-FileCopyrightText: 2005 Isaac Clerencia <isaac@warp.es>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ALLYOURBASE_H
#define ALLYOURBASE_H

#include <KWallet>
#include <QListWidget>

#include <QPixmap>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QTreeWidget>

#define KWALLETENTRYMAGIC ((quint32) 0x6B776C65)
#define KWALLETFOLDERMAGIC ((quint32) 0x6B776C66)

enum KWalletListItemClasses {
    KWalletFolderItemClass = QTreeWidgetItem::UserType,
    KWalletContainerItemClass,
    KWalletEntryItemClass,
    KWalletUnknownClass
};

class KWalletEntryItem : public QTreeWidgetItem
{
public:
    KWalletEntryItem(KWallet::Wallet *w, QTreeWidgetItem *parent, const QString &ename);
    ~KWalletEntryItem() override;

    const QString &name() const
    {
        return m_name;
    }
    void setName(const QString &n);
    // Cancel renaming
    void restoreName();

public:
    KWallet::Wallet *_wallet;

private:
    void setText(int, const QString &) {} // forbidden
    QString m_name;
};

class KWalletContainerItem : public QTreeWidgetItem
{
public:
    KWalletContainerItem(QTreeWidgetItem *parent, const QString &name, KWallet::Wallet::EntryType entryType);
    ~KWalletContainerItem() override;

public:
    KWallet::Wallet::EntryType entryType();
    bool contains(const QString &itemKey);
    QTreeWidgetItem *getItem(const QString &itemKey);

private:
    KWallet::Wallet::EntryType _type;
};

class KWalletFolderItem : public QTreeWidgetItem
{
public:
    KWalletFolderItem(KWallet::Wallet *w, QTreeWidget *parent, const QString &name, int entries);
    ~KWalletFolderItem() override;

    virtual bool acceptDrop(const QMimeData *mime) const;

    QString name() const;
    void refresh();
    KWalletContainerItem *getContainer(KWallet::Wallet::EntryType type);
    QIcon getFolderIcon();
    bool contains(const QString &itemKey);
    QTreeWidgetItem *getItem(const QString &itemKey);
    void refreshItemsCount();

public:
    KWallet::Wallet *_wallet;

private:
    QString _name;
    int _entries;
};

class KWalletEntryList : public QTreeWidget
{
    Q_OBJECT

public:
    explicit KWalletEntryList(QWidget *parent, const QString &name = QString());
    ~KWalletEntryList() override;

    bool existsFolder(const QString &name);
    KWalletFolderItem *getFolder(const QString &name);
    void setWallet(KWallet::Wallet *w);

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

    void itemDropped(QDropEvent *e, QTreeWidgetItem *item);

private:
    static KWalletFolderItem *getItemFolder(QTreeWidgetItem *item);
    QMimeData *itemMimeData(const QTreeWidgetItem *i) const;

public:
    KWallet::Wallet *_wallet;
    QPoint _mousePos;

public Q_SLOTS:
    void selectFirstVisible();
    void refreshItemsCount();
};

class KWalletItem : public QListWidgetItem
{
public:
    KWalletItem(QListWidget *parent, const QString &walletName);
    ~KWalletItem() override;

    void setOpen(bool state);

    void processDropEvent(QDropEvent *e);

private:
    bool _open;
};

inline QDataStream &operator<<(QDataStream &str, const KWalletEntryItem &w)
{
    QString name = w.text(0);
    str << name;
    KWallet::Wallet::EntryType et = w._wallet->entryType(name);
    str << qint64(et);
    QByteArray a;
    w._wallet->readEntry(name, a);
    str << a;
    return str;
}

inline QDataStream &operator<<(QDataStream &str, const KWalletFolderItem &w)
{
    QString oldFolder = w._wallet->currentFolder();
    str << w.name();
    w._wallet->setFolder(w.name());
    const QStringList entries = w._wallet->entryList();
    for (const QString &entry : entries) {
        str << entry;
        KWallet::Wallet::EntryType et = w._wallet->entryType(entry);
        str << (qint32)et;
        QByteArray a;
        w._wallet->readEntry(entry, a);
        str << a;
    }
    w._wallet->setFolder(oldFolder);
    return str;
}

#endif
