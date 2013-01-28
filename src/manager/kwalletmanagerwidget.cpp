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

#include "kwalletmanagerwidget.h"
#include "kwalletmanagerwidgetitem.h"
#include "walletwidget.h"
#include "kwallet_interface.h"

#include <kwallet.h>

KWalletManagerWidget::KWalletManagerWidget(QWidget* parent, Qt::WindowFlags flags): 
    KPageWidget(parent)
{
    setFaceType(List);
}

KWalletManagerWidget::~KWalletManagerWidget()
{

}

void KWalletManagerWidget::updateWalletDisplay()
{
    // find out pages corresponding to deleted wallets
    const QStringList wl = KWallet::Wallet::walletList();
    WalletPagesHash::iterator p = _walletPages.begin();
    while ( p != _walletPages.end() ) {
        if ( !wl.contains(p.key()) ) {
            // remove the page corresponding to the missing wallet
            p = _walletPages.erase(p);
        }
        else {
            ++p;
        }
    }

    // update existing wallets display, e.g. icon
    WalletPagesHash::const_iterator cp = _walletPages.constBegin();
    WalletPagesHash::const_iterator cend = _walletPages.constEnd();
    for ( ; cp != cend; cp++ ) {
         cp.value()->updateWalletDisplay();
    }

    // add new wallets
    for (QStringList::const_iterator i = wl.begin(); i != wl.end(); ++i) {
        const QString& name = *i;
        if ( !_walletPages.contains(name) ) {
            KWalletManagerWidgetItem *wi = new KWalletManagerWidgetItem(this, name);
            addPage( wi );
            _walletPages.insert(*i, wi);
            // TODO show open wallet icon here
        }
    }
}

bool KWalletManagerWidget::hasWallet(const QString& name) const
{
    return _walletPages.contains(name);
}
