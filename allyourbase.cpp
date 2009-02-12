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

#include "allyourbase.h"

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kglobal.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <k3urldrag.h>
#include <kwallet.h>

#include <Q3DragObject>
#include <QFile>
#include <QApplication>
#include <Q3PtrList>
//Added by qt3to4:
#include <QPixmap>
#include <Q3StrList>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>

/****************
 *  KWalletFolderItem - ListView items to represent kwallet folders
 */
KWalletFolderItem::KWalletFolderItem(KWallet::Wallet *w, Q3ListView* parent, const QString &name, int entries)
: K3ListViewItem(parent),_wallet(w),_name(name),_entries(entries) {
	setText(0, QString("%1 (%2)").arg(_name).arg(_entries));
	setRenameEnabled(0, false);
	setDragEnabled(true);
	setDropEnabled(true);

	QPixmap pix = getFolderIcon(KIconLoader::Small);

	setPixmap(0,pix);
}

QPixmap KWalletFolderItem::getFolderIcon(KIconLoader::Group group){
	KIconLoader *loader = KIconLoader::global();
	QPixmap pix = loader->loadIcon( _name, group, 0,
			KIconLoader::DefaultState, QStringList(),0, true );
	if (pix.isNull())
		pix = loader->loadIcon( _name.toLower(), group, 0,
			KIconLoader::DefaultState, QStringList(),0, true);
	if (pix.isNull())
		pix = loader->loadIcon( "folder-red", group, 0,
			KIconLoader::DefaultState, QStringList(),0, true);
	return pix;
}

void KWalletFolderItem::refresh() {
	QString saveFolder = _wallet->currentFolder();
	_wallet->setFolder(_name);
	setText(0, QString("%1 (%2)").arg(_name).arg(_wallet->entryList().count()));
	_wallet->setFolder(saveFolder);
}

KWalletContainerItem* KWalletFolderItem::getContainer(KWallet::Wallet::EntryType type) {
	for (Q3ListViewItem *i = firstChild(); i; i = i->nextSibling()) {
		KWalletContainerItem *ci = dynamic_cast<KWalletContainerItem *>(i);
		if (!ci) {
			continue;
		}
		if (ci->type() == type) {
			return ci;
		}
	}
	return 0;
}

bool KWalletFolderItem::contains(const QString& key) {
	return (getItem(key) != 0);
}

Q3ListViewItem* KWalletFolderItem::getItem(const QString& key) {
	for (Q3ListViewItem *i = firstChild(); i; i = i->nextSibling()) {
		KWalletContainerItem *ci = dynamic_cast<KWalletContainerItem *>(i);
		if (!ci) {
			continue;
		}
		Q3ListViewItem *tmp = ci->getItem(key);
		if (tmp) {
			return tmp;
		}
	}
	return 0;
}

bool KWalletFolderItem::acceptDrop(const QMimeSource *mime) const {
	return mime->provides("application/x-kwallet-entry") ||
		mime->provides("text/uri-list");
}

int KWalletFolderItem::rtti() const {
	return KWalletFolderItemClass;
}

QString KWalletFolderItem::name() const {
	return _name;
}

KWalletFolderItem::~KWalletFolderItem() {
}

/****************
 *  KWalletContainerItem - ListView items to represent kwallet containers, i.e.
 *  passwords, maps, ...
 */
KWalletContainerItem::KWalletContainerItem(Q3ListViewItem* parent, const QString &name, KWallet::Wallet::EntryType type)
: K3ListViewItem(parent, name), _type(type) {
	setRenameEnabled(0, false);
	setDragEnabled(true);
}

KWalletContainerItem::~KWalletContainerItem() {
}

int KWalletContainerItem::rtti() const {
	return KWalletContainerItemClass;
}

KWallet::Wallet::EntryType KWalletContainerItem::type() {
	return _type;
}

bool KWalletContainerItem::contains(const QString& key) {
	return getItem(key) != 0;
}

Q3ListViewItem *KWalletContainerItem::getItem(const QString& key) {
	for (Q3ListViewItem *i = firstChild(); i; i = i->nextSibling()) {
		if (i->text(0) == key) {
			return i;
		}
	}
	return 0;
}

/****************
 *  KWalletEntryItem - ListView items to represent kwallet entries
 */
KWalletEntryItem::KWalletEntryItem(KWallet::Wallet *w, Q3ListViewItem* parent, const QString& ename)
: K3ListViewItem(parent, ename), _wallet(w), _oldName(ename) {
	setRenameEnabled(0, true);
	setDragEnabled(true);
}

int KWalletEntryItem::rtti() const {
	return KWalletEntryItemClass;
}

KWalletEntryItem::~KWalletEntryItem() {
}

/****************
 * KWalletItem - IconView items to represent wallets
 */
KWalletItem::KWalletItem(Q3IconView *parent, const QString& walletName)
: Q3IconViewItem(parent, walletName, DesktopIcon("wallet-closed")), _open(false) {
}

KWalletItem::~KWalletItem() {
}

bool KWalletItem::acceptDrop(const QMimeSource *mime) const {
	return mime->provides("application/x-kwallet-folder") ||
		mime->provides("text/uri-list");
}

void KWalletItem::setOpen(bool state) {
	if (_open != state) {
		_open = state;
		if (_open) {
			setPixmap( DesktopIcon("wallet-open") );
		} else {
			setPixmap( DesktopIcon("wallet-closed") );
		}
	}
}

static bool decodeEntry(KWallet::Wallet *_wallet, QDataStream& ds) {
	quint32 magic;
	ds >> magic;
	if (magic != KWALLETENTRYMAGIC) {
		kDebug() << "bad magic" ;
		return false;
	}
	QString name;
	QByteArray value;
	KWallet::Wallet::EntryType et;
	ds >> name;
	if (_wallet->hasEntry(name)) {
		int rc = KMessageBox::warningContinueCancel(0L, i18n("An entry by the name '%1' already exists. Would you like to continue?", name));
		if (rc == KMessageBox::Cancel) {
			return false;
		}
	}
	qint32 l;
	ds >> l;
	et = KWallet::Wallet::EntryType(l);
	ds >> value;
	_wallet->writeEntry(name, value, et);
	return true;
}

static bool decodeFolder(KWallet::Wallet *_wallet, QDataStream& ds) {
	quint32 magic;
	ds >> magic;
	if (magic != KWALLETFOLDERMAGIC) {
		kDebug() << "bad magic" ;
		return false;
	}
	QString folder;
	ds >> folder;
	if (_wallet->hasFolder(folder)) {
		int rc = KMessageBox::warningYesNoCancel(0L, i18n("A folder by the name '%1' already exists.  What would you like to do?", folder), QString(), KStandardGuiItem::cont(), KGuiItem(i18n("Replace")));
		if (rc == KMessageBox::Cancel) {
			return false;
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
		qint32 l;
		ds >> l;
		et = KWallet::Wallet::EntryType(l);
		ds >> value;
		_wallet->writeEntry(name, value, et);
	}
	return true;
}

void KWalletItem::dropped(QDropEvent *e, const Q3ValueList<Q3IconDragItem>& lst) {
	Q_UNUSED(lst);
	if (e->provides("application/x-kwallet-folder") ||
			e->provides("text/uri-list")) {

		// FIXME: don't allow the drop if the wallet name is the same

		KWallet::Wallet *_wallet = KWallet::Wallet::openWallet(text(), iconView()->topLevelWidget()->winId());
		if (!_wallet) {
			e->ignore();
			return;
		}

		QString saveFolder = _wallet->currentFolder();

		QFile file;
		QDataStream *ds = 0L;

		if (e->provides("application/x-kwallet-folder")) {
			QByteArray edata = e->encodedData("application/x-kwallet-folder");
			if (!edata.isEmpty()) {
				ds = new QDataStream(&edata, QIODevice::ReadOnly);
			}
		} else { // text/uri-list
			Q3StrList urls;
			Q3UriDrag::decode(e, urls);

			if (urls.isEmpty()) {
				e->ignore();
				return;
			}

			KUrl u(urls.first());
			if (u.fileName().isEmpty()) {
				e->ignore();
				return;
			}
			QString tmpFile;
			if (KIO::NetAccess::download(u, tmpFile, 0L)) {
				file.setFileName(tmpFile);
				file.open(QIODevice::ReadOnly);
				ds = new QDataStream(&file);
				KIO::NetAccess::removeTempFile(tmpFile);
			} else {
				KMessageBox::error(iconView(), KIO::NetAccess::lastErrorString());
			}
		}
		if (ds) {
			decodeFolder(_wallet, *ds);
			delete ds;
		}
		_wallet->setFolder(saveFolder);
		delete _wallet;

		//delete the folder from the source if we were moving
		Qt::ButtonState state = QApplication::mouseButtons();
		if (e->source() && e->source()->parent() &&
		    !strcmp(e->source()->parent()->metaObject()->className(), "KWalletEntryList") &&
			!(state & Qt::ControlModifier)) {

			KWalletEntryList *el =
				dynamic_cast<KWalletEntryList*>(e->source()->parent());
			if (el) {
				KWalletFolderItem *fi =
					dynamic_cast<KWalletFolderItem*>(el->selectedItem());
				if (fi) {
					el->_wallet->removeFolder(fi->name());
				}
			}
		}
		e->accept();
	} else {
		e->ignore();
		return;
	}
}

/****************
 *  KWalletEntryDrag - Stores data for wallet entry drags
 */
class KWalletEntryDrag : public Q3StoredDrag {
	public:
		KWalletEntryDrag(QWidget *dragSource, const char *name = 0L)
			: Q3StoredDrag("application/x-kwallet-entry", dragSource, name) {
		}

		virtual ~KWalletEntryDrag() {}
};

/****************
 *  KWalletFolderDrag - Stores data for wallet folder drags
 */
class KWalletFolderDrag : public Q3StoredDrag {
	public:
		KWalletFolderDrag(QWidget *dragSource, const char *name = 0L)
			: Q3StoredDrag("application/x-kwallet-folder", dragSource, name) {
		}

		virtual ~KWalletFolderDrag() {}
};

/****************
 *  KWalletEntryList - A listview to store wallet entries
 */
KWalletEntryList::KWalletEntryList(QWidget *parent, const char *name)
: K3ListView(parent) {
	setObjectName(name);
	addColumn(i18n("Folders"));
	setRootIsDecorated(true);
	setDefaultRenameAction(Reject);
	setAcceptDrops(true);
	setItemsMovable(false);
	setDropVisualizer(false);
	viewport()->setAcceptDrops(true);
}

KWalletEntryList::~KWalletEntryList() {
}

bool KWalletEntryList::acceptDrag(QDropEvent* e) const {
	Q3ListViewItem *i = itemAt(contentsToViewport(e->pos()));
	if (i) {
		if (e->provides("application/x-kwallet-entry") ||
				e->provides("text/uri-list")) {
			return true;
		}
	}
	if ((e->provides("application/x-kwallet-folder") &&
			e->source() != viewport()) ||
			e->provides("text/uri-list")) {
		return true;
	}
	return false;
}

//returns true if the item has been dropped successfully
void KWalletEntryList::itemDropped(QDropEvent *e, Q3ListViewItem *item) {
	bool ok = true;
	bool isEntry;
	QFile file;
	QDataStream *ds;

	KWalletEntryList *el = 0L;
	Q3ListViewItem *sel = 0L;

	//detect if we are dragging from kwallet itself
	if (e->source() && e->source()->parent() &&
	    !strcmp(e->source()->parent()->metaObject()->className(), "KWalletEntryList")) {

		el = dynamic_cast<KWalletEntryList*>(e->source()->parent());
		if (!el) {
			KMessageBox::error(this, i18n("An unexpected error occurred trying to drop the item"));
		} else
			sel = el->selectedItem();
	}

	if (e->provides("application/x-kwallet-entry")) {
		//do nothing if we are in the same folder
		if (sel && sel->parent()->parent() ==
				KWalletEntryList::getItemFolder(item)) {
			e->ignore();
			return;
		}
		isEntry = true;
		QByteArray data = e->encodedData("application/x-kwallet-entry");
		if (data.isEmpty()) {
			e->ignore();
			return;
		}
		ds = new QDataStream(&data, QIODevice::ReadOnly);
	} else if (e->provides("application/x-kwallet-folder")) {
		//do nothing if we are in the same wallet
		if (this == el) {
			e->ignore();
			return;
		}
		isEntry = false;
		QByteArray data = e->encodedData("application/x-kwallet-folder");
		if (data.isEmpty()) {
			e->ignore();
			return;
		}
		ds = new QDataStream(&data, QIODevice::ReadOnly);
	} else if (e->provides("text/uri-list")) {
		Q3StrList urls;
		Q3UriDrag::decode(e, urls);
		if (urls.isEmpty()) {
			e->ignore();
			return;
		}
		KUrl u(urls.first());
		if (u.fileName().isEmpty()) {
			e->ignore();
			return;
		}
		QString tmpFile;
		if (KIO::NetAccess::download(u, tmpFile, 0L)) {
			file.setFileName(tmpFile);
			file.open(QIODevice::ReadOnly);
			ds = new QDataStream(&file);
			//check magic to discover mime type
			quint32 magic;
			(*ds) >> magic;
			delete ds;
			if (magic == KWALLETENTRYMAGIC) {
				isEntry = true;
			} else if (magic == KWALLETFOLDERMAGIC) {
				isEntry = false;
			} else {
				kDebug() << "bad magic" ;
				e->ignore();
				return;
			}
			//set the file back to the beginning
			file.reset();
			ds = new QDataStream(&file);
			KIO::NetAccess::removeTempFile(tmpFile);
		} else {
			KMessageBox::error(this, KIO::NetAccess::lastErrorString());
			return;
		}
	} else {
		e->ignore();
		return;
	}
	Qt::ButtonState state = QApplication::mouseButtons();
	if (isEntry) {
		if (!item) {
			e->ignore();
			delete(ds);
			return;
		}
		KWalletFolderItem *fi = KWalletEntryList::getItemFolder(item);
		if (!fi) {
			KMessageBox::error(this, i18n("An unexpected error occurred trying to drop the entry"));
			delete(ds);
			e->accept();
			return;
		}
		QString saveFolder = _wallet->currentFolder();
		_wallet->setFolder(fi->name());
		ok = decodeEntry(_wallet, *ds);
		_wallet->setFolder(saveFolder);
		fi->refresh();
		delete(ds);
		//delete source if we were moving, i.e., we are dragging
		//from kwalletmanager and Control is not pressed
		if (ok && el && !(state & Qt::ControlModifier) && sel) {
			el->_wallet->removeEntry(sel->text(0));
			delete sel;
		}
		e->accept();
	} else {
		ok = decodeFolder(_wallet, *ds);
		delete ds;
		//delete source if we were moving, i.e., we are dragging
		//from kwalletmanager and Control is not pressed
		if (ok && el && !(state & Qt::ControlModifier) && sel) {
			KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem *>(sel);
			if (fi) {
				el->_wallet->removeFolder(fi->name());
				delete sel;
			} else {
				KMessageBox::error(this, i18n("An unexpected error occurred trying to delete the original folder, but the folder has been copied successfully"));
			}
		}
		e->accept();
	}
}

void KWalletEntryList::setWallet(KWallet::Wallet *w) {
	_wallet = w;
}

bool KWalletEntryList::existsFolder(const QString& name) {
	for (Q3ListViewItem *vi = firstChild(); vi; vi = vi->nextSibling()) {
		KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem *>(vi);
		if (!fi) {
			continue;
		}
		if (name == fi->name()) {
			return true;
		}
	}
	return false;
}

void KWalletEntryList::contentsDropEvent(QDropEvent *e) {
	Q3ListViewItem *i = itemAt(contentsToViewport(e->pos()));
	itemDropped(e, i);
}

void KWalletEntryList::contentsDragEnterEvent(QDragEnterEvent *e) {
	if (e->provides("application/x-kwallet-entry") ||
		e->provides("application/x-kwallet-folder") ||
		e->provides("application/uri-list")) {
		e->accept();
	} else {
		e->ignore();
	}
}

KWalletFolderItem* KWalletEntryList::getFolder(const QString& name) {
	for (Q3ListViewItem *vi = firstChild(); vi; vi = vi->nextSibling()) {
		KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem *>(vi);
		if (!fi) {
			continue;
		}
		if (name == fi->name()) {
			return fi;
		}
	}
	return 0;
}

KWalletFolderItem *KWalletEntryList::getItemFolder(Q3ListViewItem *item) {
	switch (item->rtti()) {
		case KWalletFolderItemClass:
			return dynamic_cast<KWalletFolderItem *>(item);
		case KWalletContainerItemClass:
			return dynamic_cast<KWalletFolderItem *>(item->parent());
		case KWalletEntryItemClass:
			return dynamic_cast<KWalletFolderItem *>(item->parent()->parent());
	}
	return 0;
}

/****************
 *  KWalletIconDrag - Stores the data for wallet drags
 */
class KWalletIconDrag : public Q3IconDrag {
	public:
		KWalletIconDrag(QWidget *dragSource, const char *name = 0L)
			: Q3IconDrag(dragSource, name) {
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
			QByteArray mimetype(mime);
			if (mimetype == "application/x-qiconlist") {
				return Q3IconDrag::encodedData(mime);
			} else if (mimetype == "text/uri-list") {
				QByteArray s = _urls.join("\r\n").toLatin1();
				if (_urls.count() > 0) {
					s.append("\r\n");
				}
				a.resize(s.length() + 1);
				memcpy(a.data(), s.data(), s.length() + 1);
			}
			return a;
		}

		void append(const Q3IconDragItem &item, const QRect &pr,
				const QRect &tr, const QString &url) {
			Q3IconDrag::append(item, pr, tr);
			_urls.append(url);
		}

	private:
		QStringList _urls;
};

/****************
*  *  KWalletIconView - An iconview to store wallets
*   */
KWalletIconView::KWalletIconView(QWidget *parent, const char *name)
: K3IconView(parent, name) {
	KGlobal::dirs()->addResourceType("kwallet", "share/apps/kwallet");
	connect(this, SIGNAL(dropped(QDropEvent*, const Q3ValueList<Q3IconDragItem>&)), SLOT(slotDropped(QDropEvent*, const Q3ValueList<Q3IconDragItem>&)));

        // make Return execute() the icon
        connect( this, SIGNAL( returnPressed(Q3IconViewItem *) ),
                this, SIGNAL( executed(Q3IconViewItem *)) );
}

KWalletIconView::~KWalletIconView() {
}

void KWalletIconView::slotDropped(QDropEvent *e, const Q3ValueList<Q3IconDragItem>& /*lst*/) {
	if (e->source() == viewport()) {
		e->ignore();
		return;
	}

	if (!e->provides("text/uri-list")) {
		e->ignore();
		return;
	}

	QByteArray edata = e->encodedData("text/uri-list");
	QByteArray urls = edata.data();

	if (urls.isEmpty()) {
		e->ignore();
		return;
	}

	QStringList ul = QString(urls).split("\r\n", QString::SkipEmptyParts);
	if (ul.isEmpty() || ul.first().isEmpty()) {
		e->ignore();
		return;
	}

	KUrl u(ul.first());

	if (u.fileName().isEmpty()) {
		e->ignore();
		return;
	}

	QString dest = KGlobal::dirs()->saveLocation("kwallet") + u.fileName();
	if (QFile::exists(dest)) {
		KMessageBox::sorry(viewport(), i18n("That wallet file already exists.  You cannot overwrite wallets."));
		e->ignore();
		return;
	}

	// FIXME: verify that it is a real wallet file first
	KIO::NetAccess::file_copy(u, KUrl::fromPathOrUrl(dest));
	e->accept();
}

void KWalletIconView::contentsMousePressEvent(QMouseEvent *e) {
	_mousePos = e->pos();
	if (!findItem(_mousePos)) {
		clearSelection();
	}
	K3IconView::contentsMousePressEvent( e );
}

Q3DragObject *KWalletIconView::dragObject() {
	KWalletIconDrag* id = new KWalletIconDrag(viewport(), "KWallet Drag");
	QString path = "file:" + KGlobal::dirs()->saveLocation("kwallet");
	QPoint pos = _mousePos;
	for (Q3IconViewItem *item = firstItem(); item; item = item->nextItem()) {
		if (item->isSelected()) {
			QString url = path + item->text() + ".kwl";
			Q3IconDragItem idi;
			idi.setData(url.toLocal8Bit());
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

Q3DragObject *KWalletEntryList::dragObject() {
	Q3ListViewItem *i = currentItem();

	Q3StoredDrag *sd = 0L;

	if (i->rtti() == KWalletEntryItemClass) {
		KWalletEntryItem *ei = dynamic_cast<KWalletEntryItem*>(i);
		if (!ei) {
			return 0L;
		}
		KWalletContainerItem *ci = dynamic_cast<KWalletContainerItem*>(ei->parent());
		if (!ci) {
			return 0L;
		}
		sd = new KWalletEntryDrag(viewport(), "KWallet Entry Drag");
		QByteArray a;
		QDataStream ds(&a, QIODevice::WriteOnly);

		ds.setVersion(QDataStream::Qt_3_1);
		ds << KWALLETENTRYMAGIC;
		ds << ei->text(0);
		ds << ci->type();
		QByteArray value;
		ei->_wallet->readEntry(i->text(0), value);
		ds << value;
		sd->setEncodedData(a);
	} else if (i->rtti() == KWalletFolderItemClass) {
		KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem*>(i);
		if (!fi) {
			return 0L;
		}
		sd = new KWalletFolderDrag(viewport(), "KWallet Folder Drag");
		QByteArray a;
		QDataStream ds(&a, QIODevice::WriteOnly);

		ds.setVersion(QDataStream::Qt_3_1);

		ds << KWALLETFOLDERMAGIC;
		ds << *fi;
		sd->setEncodedData(a);
	}
	return sd;
}

#include "allyourbase.moc"
