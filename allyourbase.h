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

#ifndef ALLYOURBASE_H
#define ALLYOURBASE_H

#include <kiconview.h>
#include <klistview.h>
#include <kwallet.h>

class KWalletEntryItem : public QListViewItem {
	public:
		KWalletEntryItem(KWallet::Wallet *w, QListViewItem* parent, const QString& ename);
		virtual ~KWalletEntryItem();

		const QString& oldName() { return _oldName; }
		QString currentName() { return text(0); }

		void clearOldName() { _oldName = text(0); }

	private:
		QString _oldName;
	public:
		KWallet::Wallet *_wallet;
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


class KWalletEntryList : public KListView {
	public:
		KWalletEntryList(QWidget *parent, const char *name = 0L);
		virtual ~KWalletEntryList();

	protected:
		QDragObject *dragObject();
};


class KWalletFolderItem : public QIconViewItem {
	public:
		KWalletFolderItem(KWallet::Wallet *w, QIconView *parent, const QString& folderName);
		virtual ~KWalletFolderItem();
		virtual bool acceptDrop(const QMimeSource *mime) const;

	protected:
		virtual void dropped(QDropEvent *e, const QValueList<QIconDragItem>& lst); 
		KWallet::Wallet *_wallet;
};


class KWalletItem : public QIconViewItem {
	public:
		KWalletItem(QIconView *parent, const QString& walletName);
		virtual ~KWalletItem();
};


class KWalletIconView : public KIconView {
	Q_OBJECT
	public:
		KWalletIconView(QWidget *parent, const char *name = 0L);
		virtual ~KWalletIconView();

	protected:
		QDragObject *dragObject();
};

#endif
