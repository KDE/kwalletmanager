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

#include "kwmapeditor.h"

#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kwin.h>
#include <qpushbutton.h>
#include <qtextedit.h>

KWMapEditor::KWMapEditor(QMap<QString,QString>& map, QWidget *parent, const char *name)
: QTable(0, 3, parent, name), _map(map) {
	connect(this, SIGNAL(valueChanged(int,int)), this, SIGNAL(dirty()));
	connect(this, SIGNAL(contextMenuRequested(int,int,const QPoint&)),
		this, SLOT(contextMenu(int,int,const QPoint&)));
	setSelectionMode(QTable::NoSelection);
	horizontalHeader()->setLabel(0, "");
	horizontalHeader()->setLabel(1, i18n("Key"));
	horizontalHeader()->setLabel(2, i18n("Value"));
	setColumnWidth(0, 20); // FIXME: this is arbitrary
	reload();
}

void KWMapEditor::reload() {
	unsigned row = 0;

	while ((row = numRows()) > _map.count()) {
		removeRow(row - 1);
	}

	if ((row = numRows()) < _map.count()) {
		insertRows(row, _map.count() - row);
		for (int x = row; x < numRows(); ++x) {
			QPushButton *b = new QPushButton("X", this);
			connect(b, SIGNAL(clicked()), this, SLOT(erase()));
			setCellWidget(x, 0, b);
		}
	}

	row = 0;
	for (QMap<QString,QString>::Iterator it = _map.begin(); it != _map.end(); ++it) {
		setText(row, 1, it.key());
		setText(row, 2, it.data());
		row++;
	}
}


KWMapEditor::~KWMapEditor() {
}


void KWMapEditor::erase() {
	const QObject *o = sender();
	for (int i = 0; i < numRows(); i++) {
		if (cellWidget(i, 0) == o) {
			removeRow(i);
			break;
		}
	}

	emit dirty();
}


void KWMapEditor::saveMap() {
	_map.clear();

	for (int i = 0; i < numRows(); i++) {
		_map[text(i, 1)] = text(i, 2);
	}
}


void KWMapEditor::addEntry() {
	int x = numRows();
	insertRows(x, 1);
	QPushButton *b = new QPushButton("X", this);
	connect(b, SIGNAL(clicked()), this, SLOT(erase()));
	setCellWidget(x, 0, b);
	ensureCellVisible(x, 1);
	setCurrentCell(x, 1);
	emit dirty();
}


void KWMapEditor::contextMenu(int row, int col, const QPoint& pos) {
	Q_UNUSED(row)
	Q_UNUSED(col)

	KPopupMenu *m = new KPopupMenu(this);
	m->insertItem(i18n("&New Entry"), this, SLOT(addEntry()));
	m->popup(pos);
}


class InlineEditor : public QTextEdit {
	public:
		InlineEditor(KWMapEditor *p, int row, int col) : QTextEdit(), _p(p), row(row), col(col) { setWFlags(WStyle_NoBorder | WDestructiveClose); KWin::setType(winId(), NET::Override); }
		virtual ~InlineEditor() { _p->setText(row, col, text()); }

	protected:
		virtual void focusOutEvent(QFocusEvent*) { close(); }
		virtual void keyPressEvent(QKeyEvent *e) {
			if (e->key() == Qt::Key_Escape) {
				e->accept();
				close();
			} else {
				e->ignore();
				QTextEdit::keyPressEvent(e);
			}
		}
		KWMapEditor *_p;
		int row, col;
};

QWidget *KWMapEditor::beginEdit(int row, int col, bool replace) {
	kdDebug() << "EDIT COLUMN " << col << endl;
	if (col != 2) {
		return QTable::beginEdit(row, col, replace);
	}

	QRect geo = cellGeometry(row, col);
	QTextEdit *e = new InlineEditor(this, row, col);
	e->setText(text(row, col));
	e->move(mapToGlobal(geo.topLeft()));
	e->resize(geo.width() * 2, geo.height() * 3);
	e->show();
	return e;
}


#include "kwmapeditor.moc"
