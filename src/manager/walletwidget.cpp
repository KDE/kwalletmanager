/*
   Copyright (C) 2003-2005 George Staikos <staikos@kde.org>
   Copyright (C) 2005 Isaac Clerencia <isaac@warp.es>
   Copyright (C) 2013 Valentin Rusu <kde@rusu.info>

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

#include "walletwidget.h"
#include "allyourbase.h"
#include "kwmapeditor.h"
#include "kwalleteditor.h"

#include <QCheckBox>
#include <ktreewidgetsearchline.h>

WalletWidget::WalletWidget(QWidget* parent) :
    QWidget( parent ),
    _wallet(0),
    _editor(0)
{
    setupUi( this );
}

void WalletWidget::forgetWallet()
{
    _wallet = 0;
}

void WalletWidget::startWalletEditing(KWallet::Wallet* wallet)
{
    _wallet = wallet;
    Q_ASSERT(_editor != 0);
    _editor = new KWalletEditor(this, _wallet, false, this);
}

