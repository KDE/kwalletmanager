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
#include <kiconloader.h>
#include <kicontheme.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kwallet.h>

#include <qdragobject.h>
#include <qfile.h>
#include <qptrlist.h>


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
: QIconViewItem(parent, walletName, DesktopIcon("kwalletmanager")) {
}

KWalletItem::~KWalletItem() {
}

bool KWalletItem::acceptDrop(const QMimeSource *mime) const {
	return mime->provides("application/x-kwallet-folder");
}

static void decodeFolder(KWallet::Wallet *_wallet, const QByteArray& a) {
	QDataStream ds(a, IO_ReadOnly);
	QString folder;
	ds >> folder;
	if (_wallet->hasFolder(folder)) {
		int rc = KMessageBox::warningYesNoCancel(0L, i18n("A folder by the name '%1' already exists.  What would you like to do?").arg(folder), QString::null, KStdGuiItem::cont(), i18n("Replace"));
		if (rc == KMessageBox::Cancel) {
			return;
		}
		if (rc == KMessageBox::No) {
			_wallet->removeFolder(folder);
			_wallet->createFolder(folder);
		}
	} else {
		_wallet->createFolder(folder);
	}

	_wallet->setFolder(folder);
	while (!ds.atEnd()) {
		QString name;
		QByteArray value;
		KWallet::Wallet::EntryType et;
		ds >> name;
		long l;
		ds >> l;
		et = KWallet::Wallet::EntryType(l);
		ds >> value;
		_wallet->writeEntry(name, value, et);
	}
}

void KWalletItem::dropped(QDropEvent *e, const QValueList<QIconDragItem>& lst) {
	if (!e->provides("application/x-kwallet-folder")) {
		e->ignore();
		return;
	}

	// FIXME: don't allow the drop if the wallet name is the same

	KWallet::Wallet *_wallet = KWallet::Wallet::openWallet(text());
	if (!_wallet) {
		e->ignore();
		return;
	}

	QString saveFolder = _wallet->currentFolder();

	QByteArray edata = e->encodedData("application/x-kwallet-folder");
	if (!edata.isEmpty()) {
		decodeFolder(_wallet, edata);
	}

	for (QValueList<QIconDragItem>::ConstIterator it = lst.begin();
							it != lst.end();
								++it) {
		decodeFolder(_wallet, (*it).data());
	}

	_wallet->setFolder(saveFolder);
	delete _wallet;
	e->accept();
}


/****************
 *  KWalletIconDrag - Stores the data for wallet drags
 */
class KWalletFolderDrag : public QIconDrag {
	public:
		KWalletFolderDrag(QWidget *dragSource, const char *name = 0L)
			: QIconDrag(dragSource, name) {
		}

		virtual ~KWalletFolderDrag() {}

		virtual const char *format(int i = 0) const {
			if (i == 0) {
				return "application/x-qiconlist";
			} else if (i == 1) {
				return "application/x-kwallet-folder";
			}
			return 0L;
		}

		QByteArray encodedData(const char *mime) const {
			QByteArray a;
			QCString mimetype(mime);
			if (mimetype == "application/x-qiconlist") {
				return QIconDrag::encodedData(mime);
			} else if (mimetype == "application/x-kwallet-folder") {
				QDataStream ds(a, IO_WriteOnly);
				for (KWalletFolderItem *fi = _fi.first(); fi;
							fi = _fi.next()) {
					ds << *fi;
				}
			}
			return a;
		}

		void append(const QIconDragItem &item, const QRect &pr,
				const QRect &tr, KWalletFolderItem *fi) {
			QIconDrag::append(item, pr, tr);
			_fi.append(fi);
		}

	private:
		mutable QPtrList<KWalletFolderItem> _fi;
};


/****************
 *  KWalletFolderItem - IconView items to represent folders
 */
KWalletFolderItem::KWalletFolderItem(KWallet::Wallet *w, QIconView *parent, const QString& folderName)
: QIconViewItem(parent, folderName), _wallet(w) {
	KIconLoader *loader = KGlobal::instance()->iconLoader();

	QPixmap pix = loader->loadIcon( folderName, KIcon::Desktop,
		KIcon::SizeMedium, KIcon::DefaultState, 0, true );
	if ( pix.isNull() )
		pix = loader->loadIcon( folderName.lower(), KIcon::Desktop,
			KIcon::SizeMedium, KIcon::DefaultState, 0, true );
	if ( pix.isNull() )
		pix = loader->loadIcon( "folder_red", KIcon::Desktop,
			KIcon::SizeMedium, KIcon::DefaultState, 0, true );

	setPixmap( pix );
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
	_wallet->writeEntry(name, value, et);
	_wallet->setFolder(saveFolder);
	e->accept();
}

/****************
 *  KWalletFolderIconView - An iconview to store wallet folders
 */
KWalletFolderIconView::KWalletFolderIconView(QWidget *parent, const char *name)
: KIconView(parent, name) {
	connect(this, SIGNAL(dropped(QDropEvent*, const QValueList<QIconDragItem>&)), SLOT(slotDropped(QDropEvent*, const QValueList<QIconDragItem>&)));
}

KWalletFolderIconView::~KWalletFolderIconView() {
}

void KWalletFolderIconView::contentsMousePressEvent(QMouseEvent *e) {
	_mousePos = e->pos();
	if (!findItem(_mousePos)) {
		clearSelection();
	}
	KIconView::contentsMousePressEvent( e );
}

QDragObject *KWalletFolderIconView::dragObject() {
	KWalletFolderDrag *id = new KWalletFolderDrag(viewport(), "KWallet Folder Drag");
	QPoint pos = _mousePos;
	for (QIconViewItem *item = firstItem(); item; item = item->nextItem()) {
		if (item->isSelected()) {
			QIconDragItem idi;
			KWalletFolderItem *fi = static_cast<KWalletFolderItem*>(item);

			QByteArray a;
			QDataStream ds(a, IO_WriteOnly);
			ds << *fi;
			idi.setData(a);
			id->append(idi,
				QRect(item->pixmapRect(false).topLeft() - pos,
					item->pixmapRect(false).size()),
				QRect(item->textRect(false).topLeft() - pos,
					item->textRect(false).size()),
				fi);
		}
	}

	id->setPixmap(*currentItem()->pixmap(),
			pos - currentItem()->pixmapRect(false).topLeft());

	return id;
}


void KWalletFolderIconView::slotDropped(QDropEvent *e, const QValueList<QIconDragItem>& lst) {
	if (!e->provides("application/x-kwallet-folder")) {
		e->ignore();
		return;
	}

	// FIXME: ignore the drop if the wallet name is the same
	KWallet::Wallet *_wallet = KWallet::Wallet::openWallet(_walletName);
	if (!_wallet) {
		e->ignore();
		return;
	}

	QString saveFolder = _wallet->currentFolder();

	QByteArray edata = e->encodedData("application/x-kwallet-folder");
	if (!edata.isEmpty()) {
		decodeFolder(_wallet, edata);
	}

	for (QValueList<QIconDragItem>::ConstIterator it = lst.begin();
							it != lst.end();
								++it) {
		decodeFolder(_wallet, (*it).data());
	}

	_wallet->setFolder(saveFolder);
	delete _wallet;
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
	KGlobal::dirs()->addResourceType("kwallet", "share/apps/kwallet");
	connect(this, SIGNAL(dropped(QDropEvent*, const QValueList<QIconDragItem>&)), SLOT(slotDropped(QDropEvent*, const QValueList<QIconDragItem>&)));
}

KWalletIconView::~KWalletIconView() {
}

void KWalletIconView::slotDropped(QDropEvent *e, const QValueList<QIconDragItem>& /*lst*/) {
	if (e->source() == viewport()) {
		e->ignore();
		return;
	}

	if (!e->provides("text/uri-list")) {
		e->ignore();
		return;
	}

	QByteArray edata = e->encodedData("text/uri-list");
	QCString urls = edata.data();

	QStringList ul = QStringList::split("\r\n", urls);
	if (ul.isEmpty() || ul.first().isEmpty()) {
		e->ignore();
		return;
	}

	KURL u(ul.first());
	if (u.filename().isEmpty()) {
		e->ignore();
		return;
	}

	QString dest = KGlobal::dirs()->saveLocation("kwallet") + u.filename();
	if (QFile::exists(dest)) {
		KMessageBox::sorry(viewport(), i18n("That wallet file already exists.  You cannot overwrite wallets."));
		e->ignore();
		return;
	}

	// FIXME: verify that it is a real wallet file first
	KIO::NetAccess::file_copy(u, dest);
	e->accept();
}

void KWalletIconView::contentsMousePressEvent(QMouseEvent *e) {
	_mousePos = e->pos();
	if (!findItem(_mousePos)) {
		clearSelection();
	}
	KIconView::contentsMousePressEvent( e );
}

QDragObject *KWalletIconView::dragObject() {
	KWalletIconDrag* id = new KWalletIconDrag(viewport(), "KWallet Drag");
	QString path = "file:" + KGlobal::dirs()->saveLocation("kwallet");
	QPoint pos = _mousePos;
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
