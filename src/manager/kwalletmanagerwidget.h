/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2013 Valentin Rusu <kde@rusu.info>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */
#ifndef KWALLETMANAGERWIDGET_H
#define KWALLETMANAGERWIDGET_H

#include <kpagewidget.h>

class KUrl;
class QDropEvent;
class KWalletManagerWidgetItem;

class KWalletManagerWidget : public KPageWidget {
    Q_OBJECT
public:
    explicit KWalletManagerWidget(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    virtual ~KWalletManagerWidget();

    void updateWalletDisplay(QString selectWallet = QString());
    bool hasWallet(const QString&) const;
    bool openWalletFile(const QString& path);
    bool openWallet(const QString& name);

    const QString& activeWalletName() const;

protected:
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dragMoveEvent(QDragMoveEvent *e);
    virtual void dropEvent(QDropEvent *e);

private Q_SLOTS:
    void onCurrentPageChanged(KPageWidgetItem*,KPageWidgetItem*);

private:
    bool shouldIgnoreDropEvent(const QDropEvent *e, KUrl *u) const;

    typedef QHash<QString, KWalletManagerWidgetItem*> WalletPagesHash;
    WalletPagesHash _walletPages;
};

#endif // KWALLETMANAGERWIDGET_H

class KUrl;
