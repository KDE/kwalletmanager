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



class KWalletIconDrag : public QUriDrag {
	public:
		KWalletIconDrag(const QStringList& urllist, QWidget *dragSource, const char *name = 0L)
			: QUriDrag(dragSource, name), _urls(urllist) {
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
				a = QUriDrag::encodedData(mime);
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

	private:
		QStringList _urls;
};

KWalletIconView::KWalletIconView(QWidget *parent, const char *name)
: KIconView(parent, name) {
}

KWalletIconView::~KWalletIconView() {
}

QDragObject *KWalletIconView::dragObject() {
	QStringList urls;
	QString path = "file:" + KGlobal::dirs()->saveLocation("kwallet");
	for (QIconViewItem *item = firstItem(); item; item = item->nextItem()) {
		urls.append(path + item->text() + ".kwl");
	}
	return new KWalletIconDrag(urls, this, "Kwallet Drag");
}


