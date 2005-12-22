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

#include <kiconview.h>
#include <klistview.h>
#include <kwallet.h>
#include <kiconloader.h>
#include <kicontheme.h>

#define KWALLETENTRYMAGIC ((Q_UINT32) 0x6B776C65)
#define KWALLETFOLDERMAGIC ((Q_UINT32) 0x6B776C66)

enum KWalletListItemClasses {
	KWalletFolderItemClass = 1000,
	KWalletContainerItemClass,
	KWalletEntryItemClass,
	KWalletUnknownClass = 2000
};

class KWalletEntryItem : public KListViewItem {
	public:
		KWalletEntryItem(KWallet::Wallet *w, QListViewItem* parent, const QString& ename);
		virtual ~KWalletEntryItem();

		const QString& oldName() { return _oldName; }
		QString currentName() { return text(0); }

		void clearOldName() { _oldName = text(0); }
		virtual int rtti() const;

	public:
		KWallet::Wallet *_wallet;

	private:
		QString _oldName;
};

class KWalletContainerItem : public KListViewItem {
	public:
		KWalletContainerItem(QListViewItem* parent, const QString& name,
		    KWallet::Wallet::EntryType type);
		virtual ~KWalletContainerItem();

	public:
		virtual int rtti() const;
		KWallet::Wallet::EntryType type();
		bool contains(const QString& itemKey);
		QListViewItem* getItem(const QString& itemKey);

	private:
		KWallet::Wallet::EntryType _type;
};

class KWalletFolderItem : public KListViewItem {
	public:
		KWalletFolderItem(KWallet::Wallet *w, QListView* parent, 
			const QString& name, int entries);
		virtual ~KWalletFolderItem();

		virtual bool acceptDrop(const QMimeSource *mime) const;
		virtual int rtti() const;

		QString name() const;
		void refresh();
		KWalletContainerItem* getContainer(KWallet::Wallet::EntryType type);
		QPixmap getFolderIcon(KIcon::Group group);
		bool contains(const QString& itemKey);
		QListViewItem* getItem(const QString& itemKey);

	public:
		KWallet::Wallet *_wallet;

	private:
		QString _name;
		int _entries;
};

class KWalletEntryList : public KListView {
	Q_OBJECT
	public:
		KWalletEntryList(QWidget *parent, const char *name = 0L);
		virtual ~KWalletEntryList();

		bool existsFolder(const QString& name);
		KWalletFolderItem* getFolder(const QString& name);
		void contentsDropEvent(QDropEvent *e);
		void contentsDragEnterEvent(QDragEnterEvent *e);
		void setWallet(KWallet::Wallet *w);

	protected:
		void itemDropped(QDropEvent *e, QListViewItem *item);
		virtual QDragObject *dragObject();
		virtual bool acceptDrag (QDropEvent* event) const;

	private:
		static KWalletFolderItem *getItemFolder(QListViewItem *item);
	
	public:
		KWallet::Wallet *_wallet;
};

class KWalletItem : public QIconViewItem {
	public:
		KWalletItem(QIconView *parent, const QString& walletName);
		virtual ~KWalletItem();

		virtual bool acceptDrop(const QMimeSource *mime) const;

	protected:
		virtual void dropped(QDropEvent *e, const QValueList<QIconDragItem>& lst); 
};


class KWalletIconView : public KIconView {
	Q_OBJECT
	public:
		KWalletIconView(QWidget *parent, const char *name = 0L);
		virtual ~KWalletIconView();

	protected slots:
		virtual void slotDropped(QDropEvent *e, const QValueList<QIconDragItem>& lst);

	protected:
		virtual QDragObject *dragObject();
		virtual void contentsMousePressEvent(QMouseEvent *e);
		QPoint _mousePos;
};


inline QDataStream& operator<<(QDataStream& str, const KWalletEntryItem& w) {
	QString name = w.text(0);
	str << name;
	KWallet::Wallet::EntryType et = w._wallet->entryType(name);
	str << long(et);
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
	for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
		str << *it;
		KWallet::Wallet::EntryType et = w._wallet->entryType(*it);
		str << long(et);
		QByteArray a;
		w._wallet->readEntry(*it, a);
		str << a;
	}
	w._wallet->setFolder(oldFolder);
	return str;
}

#endif
