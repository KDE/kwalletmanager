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
#include <kstddirs.h>

#include <qstrlist.h>
#include <qdragobject.h>


KWalletEntryItem::KWalletEntryItem(QListViewItem* parent, const QString& ename)
: QListViewItem(parent, ename), _oldName(ename) {
	setRenameEnabled(0, true);
}

KWalletEntryItem::~KWalletEntryItem() {
}


KWalletItem::KWalletItem(QIconView *parent, const QString& walletName) 
: QIconViewItem(parent, walletName) {
}

KWalletItem::~KWalletItem() {
}



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

#include "allyourbase.moc"
