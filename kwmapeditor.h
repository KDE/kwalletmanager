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

#ifndef KWMAPEDITOR_H
#define KWMAPEDITOR_H

#include <QMap>

#include <QTableWidget>

class KAction;
class KActionCollection;

class KWMapEditor : public QTableWidget {
	Q_OBJECT

	public:
		KWMapEditor(QMap<QString,QString>& map, QWidget *parent = 0);
		virtual ~KWMapEditor();

	public slots:
		void reload();
		void saveMap();
		void erase();
		void contextMenu(const QPoint& pos);
		void addEntry();
		void emitDirty();

	private slots:
		void copy();

	signals:
		void dirty();

	private:
		QMap<QString,QString>& _map;
		int _contextRow;
		KActionCollection *_ac;
		KAction *_copyAct;
};

#endif
