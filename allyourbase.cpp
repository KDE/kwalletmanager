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

#include "allyourbase.h"

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kwallet.h>

#include <qstrlist.h>
#include <qdragobject.h>


/****************
 *  KWalletEntryItem - ListView items to represent kwallet entries
 */
KWalletEntryItem::KWalletEntryItem(KWallet::Wallet *w, QListViewItem* parent, const QString& ename)
: QListViewItem(parent, ename), _oldName(ename), _wallet(w) {
	setRenameEnabled(0, true);
	setDragEnabled(true);
}

KWalletEntryItem::~KWalletEntryItem() {
}


/****************
 *  KWalletItem - IconView items to represent wallets
 */
KWalletItem::KWalletItem(QIconView *parent, const QString& walletName) 
: QIconViewItem(parent, walletName) {
}

KWalletItem::~KWalletItem() {
}



/****************
 *  KWalletFolderItem - IconView items to represent folders
 */
KWalletFolderItem::KWalletFolderItem(KWallet::Wallet *w, QIconView *parent, const QString& folderName) 
: QIconViewItem(parent, folderName), _wallet(w) {
}

KWalletFolderItem::~KWalletFolderItem() {
}

bool KWalletFolderItem::acceptDrop(const QMimeSource *mime) const {
	return mime->provides("application/x-kwallet-entry");
}

void KWalletFolderItem::dropped(QDropEvent *e, const QValueList<QIconDragItem>& lst) {
	Q_UNUSED(lst)
	if (!e->provides("application/x-kwallet-entry")) {
		e->ignore();
		return;
	}

	QByteArray data = e->encodedData("application/x-kwallet-entry");
	if (data.isEmpty()) {
		e->ignore();
		return;
	}
	QString saveFolder = _wallet->currentFolder();
	_wallet->setFolder(text());
	QString name;
	QByteArray value;
	KWallet::Wallet::EntryType et;
	QDataStream ds(data, IO_ReadOnly);
	ds >> name;
	long l;
	ds >> l;
	et = KWallet::Wallet::EntryType(l);
	ds >> value;
	if (et == KWallet::Wallet::Map) {
		QMap<QString,QString> m;
		if (!value.isEmpty()) {
			QDataStream ds2(value, IO_ReadOnly);
			ds2 >> m;
		}
		_wallet->writeMap(name, m);
	} else if (et == KWallet::Wallet::Password) {
		QString p;
		if (!value.isEmpty()) {
			QDataStream ds2(value, IO_ReadOnly);
			ds2 >> p;
		}
		_wallet->writePassword(name, p);
	} else {
		_wallet->writeEntry(name, value);
	}

	_wallet->setFolder(saveFolder);
	e->accept();
}

/****************
 *  KWalletIconDrag - Stores the data for wallet drags
 */
class KWalletIconDrag : public QIconDrag {
	public:
		KWalletIconDrag(QWidget *dragSource, const char *name = 0L)
			: QIconDrag(dragSource, name) {
		}

		virtual ~KWalletIconDrag() {}

		virtual const char *format(int i = 0) const {
			if (i == 0) {
				return "application/x-qiconlist";
			} else if (i == 1) {
				return "text/uri-list";
			}
			return 0L;
		}

		QByteArray encodedData(const char *mime) const {
			QByteArray a;
			QCString mimetype(mime);
			if (mimetype == "application/x-qiconlist") {
				return QIconDrag::encodedData(mime);
			} else if (mimetype == "text/uri-list") {
				QCString s = _urls.join("\r\n").latin1();
				if(_urls.count() > 0) {
					s.append("\r\n");
				}
				a.resize(s.length() + 1);
				memcpy(a.data(), s.data(), s.length() + 1);
			}
			return a;
		}

		void append(const QIconDragItem &item, const QRect &pr,
				const QRect &tr, const QString &url) {
			QIconDrag::append(item, pr, tr);
			_urls.append(url);
		}

	private:
		QStringList _urls;
};


/****************
 *  KWalletIconView - An iconview to store wallets
 */
KWalletIconView::KWalletIconView(QWidget *parent, const char *name)
: KIconView(parent, name) {
}

KWalletIconView::~KWalletIconView() {
}

QDragObject *KWalletIconView::dragObject() {
	KWalletIconDrag *id = new KWalletIconDrag(viewport(), "KWallet Drag");
	QString path = "file:" + KGlobal::dirs()->saveLocation("kwallet");
	QPoint pos = mapFromGlobal(QCursor::pos());
	for (QIconViewItem *item = firstItem(); item; item = item->nextItem()) {
		if (item->isSelected()) {
			QString url = path + item->text() + ".kwl";
			QIconDragItem idi;
			idi.setData(url.local8Bit());
			id->append(idi,
				QRect(item->pixmapRect(false).topLeft() - pos,
					item->pixmapRect(false).size()),
				QRect(item->textRect(false).topLeft() - pos,
					item->textRect(false).size()),
				url);
		}
	}

	id->setPixmap(*currentItem()->pixmap(),
			pos - currentItem()->pixmapRect(false).topLeft());

	return id;
}



/****************
 *  KWalletEntryDrag - Stores data for wallet entry drags
 */
class KWalletEntryDrag : public QStoredDrag {
	public:
		KWalletEntryDrag(QWidget *dragSource, const char *name = 0L)
			: QStoredDrag("application/x-kwallet-entry", dragSource, name) {
		}

		virtual ~KWalletEntryDrag() {}
};


/****************
 *  KWalletEntryList - A listview to store wallet entries
 */
KWalletEntryList::KWalletEntryList(QWidget *parent, const char *name)
: KListView(parent, name) {
	addColumn(i18n("Folder Entry"));
	setRootIsDecorated(true);
	setDefaultRenameAction(Reject);
}

KWalletEntryList::~KWalletEntryList() {
}

QDragObject *KWalletEntryList::dragObject() {
	KWalletEntryDrag *ed = 0L;
	QListViewItem *i = currentItem();

	if (i) {
		KWalletEntryItem *ei = dynamic_cast<KWalletEntryItem*>(i);
		if (!ei) {
			return 0L;
		}
		ed = new KWalletEntryDrag(viewport(), "KWallet Entry Drag");
		QByteArray a;
		QDataStream ds(a, IO_WriteOnly);
		ds << *ei;
		ed->setEncodedData(a);
	}

	return ed;
}

#include "allyourbase.moc"
