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

#ifndef ALLYOURBASE_H
#define ALLYOURBASE_H

#include <kwallet.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klistwidget.h>
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

class KWalletEntryItem : public QTreeWidgetItem {
	public:
		KWalletEntryItem(KWallet::Wallet *w, QTreeWidgetItem* parent, const QString& ename);
		virtual ~KWalletEntryItem();

		const QString& name() const { return m_name; }
		void setName(const QString& n);
		// Cancel renaming
		void restoreName();

	public:
		KWallet::Wallet *_wallet;

	private:
		void setText(int, const QString&) {} // forbidden
		QString m_name;
};

class KWalletContainerItem : public QTreeWidgetItem {
	public:
		KWalletContainerItem(QTreeWidgetItem* parent, const QString& name, KWallet::Wallet::EntryType entryType);
		virtual ~KWalletContainerItem();

	public:
		KWallet::Wallet::EntryType entryType();
		bool contains(const QString& itemKey);
		QTreeWidgetItem* getItem(const QString& itemKey);

	private:
		KWallet::Wallet::EntryType _type;
};

class KWalletFolderItem : public QTreeWidgetItem {
	public:
		KWalletFolderItem(KWallet::Wallet *w, QTreeWidget* parent, const QString& name, int entries);
		virtual ~KWalletFolderItem();

		virtual bool acceptDrop(const QMimeSource *mime) const;

		QString name() const;
		void refresh();
		KWalletContainerItem* getContainer(KWallet::Wallet::EntryType type);
		QPixmap getFolderIcon(KIconLoader::Group group);
		bool contains(const QString& itemKey);
		QTreeWidgetItem* getItem(const QString& itemKey);
        void refreshItemsCount();

	public:
		KWallet::Wallet *_wallet;

	private:
		QString _name;
		int _entries;
};

class KWalletEntryList : public QTreeWidget {
	Q_OBJECT

	public:
		explicit KWalletEntryList(QWidget *parent, const char *name = 0L);
		virtual ~KWalletEntryList();

		bool existsFolder(const QString& name);
		KWalletFolderItem* getFolder(const QString& name);
		void setWallet(KWallet::Wallet *w);

	protected:
		virtual void dragEnterEvent(QDragEnterEvent *e);
		virtual void dragMoveEvent(QDragMoveEvent *e);
		virtual void dropEvent(QDropEvent *e);
		virtual void mousePressEvent(QMouseEvent *e);
		virtual void mouseMoveEvent(QMouseEvent *e);
		
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

class KWalletItem : public QListWidgetItem {
	public:
		KWalletItem(QListWidget *parent, const QString& walletName);
		virtual ~KWalletItem();

		void setOpen(bool state);
		
		void processDropEvent(QDropEvent *e);

	private:
		bool _open;
};

inline QDataStream& operator<<(QDataStream& str, const KWalletEntryItem& w) {
	QString name = w.text(0);
	str << name;
	KWallet::Wallet::EntryType et = w._wallet->entryType(name);
	str << qint64(et);
	QByteArray a;
	w._wallet->readEntry(name, a);
	str << a;
	return str;
}

inline QDataStream& operator<<(QDataStream& str, const KWalletFolderItem& w) {
	QString oldFolder = w._wallet->currentFolder();
	str << w.name();
	w._wallet->setFolder(w.name());
	QStringList entries = w._wallet->entryList();
	foreach (const QString &entry, entries) {
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
