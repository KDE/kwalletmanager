/*
    SPDX-FileCopyrightText: 2003, 2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWMAPEDITOR_H
#define KWMAPEDITOR_H

#include <QMap>

#include <QTableWidget>

class KActionCollection;

class KWMapEditor : public QTableWidget
{
    Q_OBJECT

public:
    explicit KWMapEditor(QMap<QString, QString> &map, QWidget *parent = nullptr);
    ~KWMapEditor() override;

public Q_SLOTS:
    void reload();
    void saveMap();
    void erase();
    void contextMenu(const QPoint &pos);
    void addEntry();
    void emitDirty();

private Q_SLOTS:
    void copy();

Q_SIGNALS:
    void dirty();

private:
    QMap<QString, QString> &_map;
    int _contextRow;
    KActionCollection *_ac;
    QAction *_copyAct;
};

#endif
