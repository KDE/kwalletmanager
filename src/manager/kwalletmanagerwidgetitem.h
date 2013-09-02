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
#ifndef KWALLETMANAGERWIDGETITEM_H
#define KWALLETMANAGERWIDGETITEM_H

#include <kpagewidgetmodel.h>

class WalletControlWidget;
class KWalletManagerWidgetItem : public KPageWidgetItem {
    Q_OBJECT
public:
    KWalletManagerWidgetItem(QWidget* widgetParent, const QString& walletName);

    void updateWalletDisplay();
    const QString& walletName() const { return _walletName; }
    bool openWallet();

private:
    WalletControlWidget *_controlWidget;
    QString             _walletName;
};

#endif // KWALLETMANAGERWIDGETITEM_H
