/*
   Copyright (C) 2003,2004 George Staikos <staikos@kde.org>

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

#include "kwmapeditor.h"
#include <kaction.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandardaction.h>
#include <kwindowsystem.h>
#include <QPointer>
#include <QApplication>
#include <QClipboard>
#include <QToolButton>
#include <ktextedit.h>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QItemDelegate>

class InlineEditor : public KTextEdit {
	public:
	InlineEditor(KWMapEditor *p) : KTextEdit(), _p(p) {
		setAttribute(Qt::WA_DeleteOnClose);
		setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
		setCheckSpellingEnabled(false);
		connect(p, SIGNAL(destroyed()), SLOT(close()));
	}

	protected:
		virtual void focusOutEvent(QFocusEvent *e) {
			if (e->reason() == Qt::PopupFocusReason) {
				// TODO: It seems we only get here if we're disturbed
				// by our own popup. this needs some clearance though.
				return;
			}

			close();
		}
		virtual void keyPressEvent(QKeyEvent *e) {
			if (e->key() == Qt::Key_Escape) {
				e->accept();
				close();
			} else {
				e->ignore();
				KTextEdit::keyPressEvent(e);
			}
		}
		virtual void contextMenuEvent( QContextMenuEvent *event )
		{
		   QMenu *menu = createStandardContextMenu();
		   popup = menu;
		   popup->exec( event->globalPos() );
		   delete popup;
		}
		QPointer<KWMapEditor> _p;
		QPointer<QMenu> popup;
};

class KWMapEditorDelegate : public QItemDelegate
{
	public:
		KWMapEditorDelegate(KWMapEditor *parent) : QItemDelegate(parent)
		{
		}
		
		QWidget *createEditor(QWidget *parentWidget, const QStyleOptionViewItem &option, const QModelIndex &index) const
		{
			if (index.column() != 2) {
				return QItemDelegate::createEditor(parentWidget, option, index);
			}

			KWMapEditor *mapEditor = static_cast<KWMapEditor*>(parent());
			return new InlineEditor(mapEditor);
		}
		
		void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
		{
			if (dynamic_cast<InlineEditor*>(editor))
			{
				KWMapEditor *mapEditor = static_cast<KWMapEditor*>(parent());
				const QRect geo = mapEditor->visualRect(index);
				editor->move(mapEditor->mapToGlobal(geo.topLeft()));
				editor->resize(geo.width() * 2, geo.height() * 3);
			}
			else
			{
				QItemDelegate::updateEditorGeometry(editor, option, index);
			}
		}
		
		void setEditorData(QWidget *editor, const QModelIndex &index) const
		{
			InlineEditor *e = dynamic_cast<InlineEditor*>(editor);
			if (e)
			{
				KWMapEditor *mapEditor = static_cast<KWMapEditor*>(parent());
				e->setText(mapEditor->item(index.row(), index.column())->text());
			}
			else
			{
				QItemDelegate::setEditorData(editor, index);
			}
		}
		
		void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
		{
			InlineEditor *e = dynamic_cast<InlineEditor*>(editor);
			if (e)
			{
				KWMapEditor *mapEditor = static_cast<KWMapEditor*>(parent());
				mapEditor->item(index.row(), index.column())->setText(e->toPlainText());
			}
			else
			{
				QItemDelegate::setModelData(editor, model, index);
			}
		}
};

KWMapEditor::KWMapEditor(QMap<QString,QString>& map, QWidget *parent)
: QTableWidget(0, 3, parent), _map(map) {
	setItemDelegate(new KWMapEditorDelegate(this));
	_ac = new KActionCollection(this);
	_copyAct = KStandardAction::copy(this, SLOT(copy()), _ac);
	connect(this, SIGNAL(itemChanged(QTableWidgetItem*)), this, SIGNAL(dirty()));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
	setSelectionMode(NoSelection);
	setHorizontalHeaderLabels(QStringList() << QString() << i18n("Key") << i18n("Value"));
	setContextMenuPolicy(Qt::CustomContextMenu);
	reload();
}

void KWMapEditor::reload() {
	int row = 0;

	while ((row = rowCount()) > _map.count()) {
		removeRow(row - 1);
	}

	if ((row = rowCount()) < _map.count()) {
		setRowCount(_map.count());
		for (int x = row; x < rowCount(); ++x) {
			QToolButton *b = new QToolButton(this);
			b->setIcon(KIcon("edit-delete"));
			b->setToolTip(i18n("Delete Entry"));
			connect(b, SIGNAL(clicked()), this, SLOT(erase()));
			setCellWidget(x, 0, b);
			if (columnWidth(0) != b->sizeHint().width()) {
				setColumnWidth(0, b->sizeHint().width());
			}
			setItem(x, 1, new QTableWidgetItem());
			setItem(x, 2, new QTableWidgetItem());
		}
	}

	row = 0;
	for (QMap<QString,QString>::Iterator it = _map.begin(); it != _map.end(); ++it) {
		item(row, 1)->setText(it.key());
		item(row, 2)->setText(it.value());
		row++;
	}
}


KWMapEditor::~KWMapEditor() {
}


void KWMapEditor::erase() {
	const QObject *o = sender();
	for (int i = 0; i < rowCount(); i++) {
		if (cellWidget(i, 0) == o) {
			removeRow(i);
			break;
		}
	}

	emit dirty();
}


void KWMapEditor::saveMap() {
	_map.clear();

	for (int i = 0; i < rowCount(); i++) {
		_map[item(i, 1)->text()] = item(i, 2)->text();
	}
}


void KWMapEditor::addEntry() {
	int x = rowCount();
	insertRow(x);
	QToolButton *b = new QToolButton(this);
	b->setIcon(KIcon("edit-delete"));
	b->setToolTip(i18n("Delete Entry"));
	connect(b, SIGNAL(clicked()), this, SLOT(erase()));
	setCellWidget(x, 0, b);
	setItem(x, 1, new QTableWidgetItem());
	setItem(x, 2, new QTableWidgetItem());
	scrollToItem(item(x, 1));
	setCurrentCell(x, 1);
	emit dirty();
}


void KWMapEditor::emitDirty() {
	emit dirty();
}


void KWMapEditor::contextMenu(const QPoint& pos) {
	QTableWidgetItem *twi = itemAt(pos);
	_contextRow = row(twi);
	KMenu *m = new KMenu(this);
	m->addAction( i18n("&New Entry" ), this, SLOT(addEntry()));
	m->addAction( _copyAct );
	m->popup(mapToGlobal(pos));
}


void KWMapEditor::copy() {
	QTableWidgetItem *twi = item(_contextRow, 2);
	if (twi)
		QApplication::clipboard()->setText(twi->text());
}

#include "kwmapeditor.moc"
