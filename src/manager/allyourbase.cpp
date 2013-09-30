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
#include <kwallet.h>

#include <QFile>
#include <QApplication>
//Added by qt3to4:
#include <QPixmap>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>

/****************
 *  KWalletFolderItem - ListView items to represent kwallet folders
 */
KWalletFolderItem::KWalletFolderItem(KWallet::Wallet *w, QTreeWidget* parent, const QString &name, int entries)
: QTreeWidgetItem(parent, KWalletFolderItemClass),_wallet(w),_name(name),_entries(entries) {
    setText(0, QString::fromLatin1("%1 (%2)").arg(_name).arg(_entries));
	setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
	setIcon(0, getFolderIcon(KIconLoader::Small));
}

QPixmap KWalletFolderItem::getFolderIcon(KIconLoader::Group group){
	KIconLoader *loader = KIconLoader::global();
	QPixmap pix = loader->loadIcon( _name, group, 0,
			KIconLoader::DefaultState, QStringList(),0, true );
	if (pix.isNull())
		pix = loader->loadIcon( _name.toLower(), group, 0,
			KIconLoader::DefaultState, QStringList(),0, true);
	if (pix.isNull())
		pix = loader->loadIcon( QLatin1String( "folder-red" ), group, 0,
			KIconLoader::DefaultState, QStringList(),0, true);
	return pix;
}

void KWalletFolderItem::refresh() {
	const QString saveFolder = _wallet->currentFolder();
	_wallet->setFolder(_name);
	setText(0, QString::fromLatin1("%1 (%2)").arg(_name).arg(_wallet->entryList().count()));
	_wallet->setFolder(saveFolder);
}

void KWalletFolderItem::refreshItemsCount()
{
    int visibleLeafCount =0;
    for (int i=0; i < childCount(); i++) {
        QTreeWidgetItem* wi = child(i);
        if (wi->childCount()) {
            for (int l=0; l < wi->childCount(); l++) {
                QTreeWidgetItem* li = wi->child(l);
                if (!li->isHidden())
                    visibleLeafCount++;
            }
        }
    }
    setText(0, QString::fromLatin1("%1 (%2)").arg(_name).arg(visibleLeafCount));
}

KWalletContainerItem* KWalletFolderItem::getContainer(KWallet::Wallet::EntryType type) {
	for (int i = 0; i < childCount(); ++i) {
		KWalletContainerItem *ci = dynamic_cast<KWalletContainerItem *>(child(i));
		if (!ci) {
			continue;
		}
		if (ci->entryType() == type) {
			return ci;
		}
	}
	return 0;
}

bool KWalletFolderItem::contains(const QString& key) {
	return (getItem(key) != 0);
}

QTreeWidgetItem* KWalletFolderItem::getItem(const QString& key) {
	for (int i = 0; i < childCount(); ++i) {
		KWalletContainerItem *ci = dynamic_cast<KWalletContainerItem *>(child(i));
		if (!ci) {
			continue;
		}
		QTreeWidgetItem *tmp = ci->getItem(key);
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

QString KWalletFolderItem::name() const {
	return _name;
}

KWalletFolderItem::~KWalletFolderItem() {
}

/****************
 *  KWalletContainerItem - ListView items to represent kwallet containers, i.e.
 *  passwords, maps, ...
 */
KWalletContainerItem::KWalletContainerItem(QTreeWidgetItem* parent, const QString &name, KWallet::Wallet::EntryType entryType)
: QTreeWidgetItem(parent, QStringList() << name, KWalletContainerItemClass), _type(entryType) {
	setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
}

KWalletContainerItem::~KWalletContainerItem() {
}

KWallet::Wallet::EntryType KWalletContainerItem::entryType() {
	return _type;
}

bool KWalletContainerItem::contains(const QString& key) {
	return getItem(key) != 0;
}

QTreeWidgetItem *KWalletContainerItem::getItem(const QString& key) {
	for (int i = 0; i < childCount(); ++i) {
		KWalletEntryItem* entryItem = dynamic_cast<KWalletEntryItem *>(child(i));
		if (entryItem && entryItem->name() == key) {
			return entryItem;
		}
	}
	return 0;
}

/****************
 *  KWalletEntryItem - ListView items to represent kwallet entries
 */
KWalletEntryItem::KWalletEntryItem(KWallet::Wallet *w, QTreeWidgetItem* parent, const QString& ename)
: QTreeWidgetItem(parent, QStringList() << ename, KWalletEntryItemClass), _wallet(w), m_name(ename) {
	setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
}

KWalletEntryItem::~KWalletEntryItem() {
}

void KWalletEntryItem::setName(const QString& n)
{
	m_name = n;
	QTreeWidgetItem::setText(0, n);
}

void KWalletEntryItem::restoreName()
{
	QTreeWidgetItem::setText(0, m_name);
}


/****************
 * KWalletItem - IconView items to represent wallets
 */
KWalletItem::KWalletItem(QListWidget *parent, const QString& walletName)
: QListWidgetItem(DesktopIcon(QLatin1String( "wallet-closed" )), walletName, parent), _open(false) {
	setFlags(flags() | Qt::ItemIsDropEnabled);
}

KWalletItem::~KWalletItem() {
}

void KWalletItem::setOpen(bool state) {
	if (_open != state) {
		_open = state;
		if (_open) {
			setIcon( DesktopIcon(QLatin1String( "wallet-open" )) );
		} else {
			setIcon( DesktopIcon(QLatin1String( "wallet-closed" )) );
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

void KWalletItem::processDropEvent(QDropEvent *e) {
	if (e->provides("application/x-kwallet-folder") ||
	    e->provides("text/uri-list")) {
		// FIXME: don't allow the drop if the wallet name is the same
		KWallet::Wallet *_wallet = KWallet::Wallet::openWallet(text(), listWidget()->topLevelWidget()->winId());
		if (!_wallet) {
			e->ignore();
			return;
		}

		const QString saveFolder = _wallet->currentFolder();

		QDataStream *ds = 0L;

		if (e->provides("application/x-kwallet-folder")) {
			QByteArray edata = e->encodedData("application/x-kwallet-folder");
			if (!edata.isEmpty()) {
				ds = new QDataStream(&edata, QIODevice::ReadOnly);
			}
		} else { // text/uri-list
			const QList<QUrl> urls = e->mimeData()->urls();
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
				QFile file;
				file.setFileName(tmpFile);
				file.open(QIODevice::ReadOnly);
				ds = new QDataStream(&file);
				KIO::NetAccess::removeTempFile(tmpFile);
			} else {
				KMessageBox::error(listWidget(), KIO::NetAccess::lastErrorString());
			}
		}
		if (ds) {
			decodeFolder(_wallet, *ds);
			delete ds;
		}
		_wallet->setFolder(saveFolder);
		delete _wallet;

		//delete the folder from the source if we were moving
		Qt::MouseButtons state = QApplication::mouseButtons();
		if (e->source() && e->source()->parent() &&
		    !strcmp(e->source()->parent()->metaObject()->className(), "KWalletEntryList") &&
			!(state & Qt::ControlModifier)) {

			KWalletEntryList *el =
				dynamic_cast<KWalletEntryList*>(e->source()->parent());
			if (el) {
				KWalletFolderItem *fi =
					dynamic_cast<KWalletFolderItem*>(el->currentItem());
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
 *  KWalletEntryList - A listview to store wallet entries
 */
KWalletEntryList::KWalletEntryList(QWidget *parent, const char *name)
: QTreeWidget(parent),
    _wallet(0) {
	setObjectName( QLatin1String( name ) );
	setColumnCount(1);
	setHeaderLabel(i18n("Folders"));
	setRootIsDecorated(true);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDragDropMode(DragDrop);
    setSelectionMode(SingleSelection);
}

KWalletEntryList::~KWalletEntryList() {
}

//returns true if the item has been dropped successfully
void KWalletEntryList::itemDropped(QDropEvent *e, QTreeWidgetItem *item) {
	bool ok = true;
	bool isEntry;
	QFile file;
	QDataStream *ds;

	KWalletEntryList *el = 0L;
	QTreeWidgetItem *sel = 0L;

	//detect if we are dragging from kwallet itself
	kDebug() << e->source() << e->source()->metaObject()->className();
	if (e->source() && !strcmp(e->source()->metaObject()->className(), "KWalletEntryList")) {

		el = dynamic_cast<KWalletEntryList*>(e->source());
		if (!el) {
			KMessageBox::error(this, i18n("An unexpected error occurred trying to drop the item"));
		} else
			sel = el->currentItem();
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
		const QList<QUrl> urls = e->mimeData()->urls();
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
	Qt::MouseButtons state = QApplication::mouseButtons();
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
	for (int i = 0; i < topLevelItemCount(); ++i) {
		KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem *>(topLevelItem(i));
		if (!fi) {
			continue;
		}
		if (name == fi->name()) {
			return true;
		}
	}
	return false;
}

QMimeData *KWalletEntryList::itemMimeData(const QTreeWidgetItem *i) const {
	QMimeData *sd = 0L;
	if (i->type() == KWalletEntryItemClass) {
		const KWalletEntryItem *ei = dynamic_cast<const KWalletEntryItem*>(i);
		if (!ei) {
			return 0L;
		}
		KWalletContainerItem *ci = dynamic_cast<KWalletContainerItem*>(ei->parent());
		if (!ci) {
			return 0L;
		}
		sd = new QMimeData();
		QByteArray a;
		QDataStream ds(&a, QIODevice::WriteOnly);

		ds.setVersion(QDataStream::Qt_3_1);
		ds << KWALLETENTRYMAGIC;
		ds << ei->text(0);
		ds << ci->entryType();
		QByteArray value;
		ei->_wallet->readEntry(i->text(0), value);
		ds << value;
		sd->setData("application/x-kwallet-entry", a);
	} else if (i->type() == KWalletFolderItemClass) {
		const KWalletFolderItem *fi = dynamic_cast<const KWalletFolderItem*>(i);
		if (!fi) {
			return 0L;
		}
		sd = new QMimeData();
		QByteArray a;
		QDataStream ds(&a, QIODevice::WriteOnly);

		ds.setVersion(QDataStream::Qt_3_1);

		ds << KWALLETFOLDERMAGIC;
		ds << *fi;
		sd->setData("application/x-kwallet-folder", a);
	}
	return sd;
}

void KWalletEntryList::mousePressEvent(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton)
		_mousePos = e->pos();
	QTreeWidget::mousePressEvent( e );
}

void KWalletEntryList::mouseMoveEvent(QMouseEvent *e) {
	if (!(e->buttons() & Qt::LeftButton))
		return;
	if ((e->pos() - _mousePos).manhattanLength() < QApplication::startDragDistance())
		return;
	
	const QTreeWidgetItem *item = itemAt(_mousePos);
	if (!item || !item->isSelected())
		return;
	
	QMimeData *mimeData = itemMimeData(item);
	if (mimeData) {
		QDrag *drag = new QDrag(this);
		drag->setMimeData(mimeData);
		drag->setHotSpot(QPoint(0,0));
		drag->exec();
	}
}

void KWalletEntryList::dropEvent(QDropEvent *e) {
	QTreeWidgetItem *i = itemAt(e->pos());
	itemDropped(e, i);
}

void KWalletEntryList::dragEnterEvent(QDragEnterEvent *e) {
	e->accept();
}

void KWalletEntryList::dragMoveEvent(QDragMoveEvent *e) {
	QTreeWidgetItem *i = itemAt(e->pos());
	e->ignore();
	if (i) {
		if (e->provides("application/x-kwallet-entry") ||
				e->provides("text/uri-list")) {
			e->accept();
		}
	}
	if ((e->provides("application/x-kwallet-folder") &&
			e->source() != viewport()) ||
			e->provides("text/uri-list")) {
		e->accept();
	}
}

KWalletFolderItem* KWalletEntryList::getFolder(const QString& name) {
	for (int i = 0; i < topLevelItemCount(); ++i) {
		KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem *>(topLevelItem(i));
		if (!fi) {
			continue;
		}
		if (name == fi->name()) {
			return fi;
		}
	}
	return 0;
}

KWalletFolderItem *KWalletEntryList::getItemFolder(QTreeWidgetItem *item) {
	switch (item->type()) {
		case KWalletFolderItemClass:
			return dynamic_cast<KWalletFolderItem *>(item);
		case KWalletContainerItemClass:
			return dynamic_cast<KWalletFolderItem *>(item->parent());
		case KWalletEntryItemClass:
			return dynamic_cast<KWalletFolderItem *>(item->parent()->parent());
	}
	return 0;
}

void KWalletEntryList::selectFirstVisible()
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        QTreeWidgetItem *item = *it++;
        if (!item->isHidden()) {
            // if it's a leaf, then select it and quit
            if (item->childCount() ==0) {
//                 kDebug() << "selecting " << item->text(0);
                setCurrentItem(item);
                break;
            }
        }
    }
}

void KWalletEntryList::refreshItemsCount()
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        QTreeWidgetItem *item = *it++;
        KWalletFolderItem *fi = dynamic_cast< KWalletFolderItem* >(item);
        if (fi) {
            fi->refreshItemsCount();
        }
    }
}

class ReturnPressedFilter : public QObject
{
	public:
		ReturnPressedFilter(KListWidget *parent) : QObject(parent)
		{
			parent->installEventFilter(this);
		}
	
		bool eventFilter(QObject * /*watched*/, QEvent *event)
		{
			if (event->type () == QEvent::KeyPress)
			{
				QKeyEvent *ke = static_cast<QKeyEvent *>(event);
				if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return)
				{
					KListWidget *p = static_cast<KListWidget*>(parent());
					QMetaObject::invokeMethod(p, "executed", Q_ARG(QListWidgetItem*, p->currentItem()));
					return true;
				}
			}
			return false;
		}
};

#include "allyourbase.moc"
