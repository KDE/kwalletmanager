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
#include "kwallet_interface.h"

#include <kwallet.h>
#include <kurl.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <klocalizedstring.h>
#include <kio/netaccess.h>
#include <QDragEnterEvent>

KWalletManagerWidget::KWalletManagerWidget(QWidget* parent, Qt::WindowFlags flags): 
    KPageWidget(parent)
{
    setFaceType(Auto);
    setAcceptDrops(true);

    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)), SLOT(onCurrentPageChanged(KPageWidgetItem*,KPageWidgetItem*)));
}

KWalletManagerWidget::~KWalletManagerWidget()
{

}

void KWalletManagerWidget::onCurrentPageChanged(KPageWidgetItem* current, KPageWidgetItem* before)
{

}

void KWalletManagerWidget::updateWalletDisplay(QString selectWallet /* = QString() */)
{
    // NOTE: this method is called upon several kwalletd events
    static bool alreadyUpdating = false;
    if (alreadyUpdating)
        return;

    alreadyUpdating = true;
    // find out pages corresponding to deleted wallets
    const QStringList wl = KWallet::Wallet::walletList();
    WalletPagesHash::iterator p = _walletPages.begin();
    while ( p != _walletPages.end() ) {
        if ( !wl.contains(p.key()) ) {
            // remove the page corresponding to the missing wallet
            removePage(p.value());
            p = _walletPages.erase(p);
        }
        else {
            ++p;
        }
    }

    // add new wallets
    for (QStringList::const_iterator i = wl.begin(); i != wl.end(); ++i) {
        const QString& name = *i;
        if ( !_walletPages.contains(name) ) {
            KWalletManagerWidgetItem *wi = new KWalletManagerWidgetItem(this, name);
            addPage( wi );
            _walletPages.insert(*i, wi);
        }
    }

    // update existing wallets display, e.g. icon
    WalletPagesHash::const_iterator cp = _walletPages.constBegin();
    WalletPagesHash::const_iterator cend = _walletPages.constEnd();
    for ( ; cp != cend; cp++ ) {
         cp.value()->updateWalletDisplay();
    }

    if (!selectWallet.isEmpty()) {
        setCurrentPage(_walletPages[selectWallet]);
    }
    alreadyUpdating = false;
}

bool KWalletManagerWidget::hasWallet(const QString& name) const
{
    return _walletPages.contains(name);
}

bool KWalletManagerWidget::openWalletFile(const QString& path)
{
    Q_ASSERT(0);
    // TODO: implement this method: add a new tab with an editor centered on a file
    return false;
}

bool KWalletManagerWidget::openWallet(const QString& name)
{
    bool result = false;
    if (_walletPages.contains(name)) {
        KWalletManagerWidgetItem *wi = _walletPages[name];
        setCurrentPage(wi);
        result = wi->openWallet();
    }
    return result;
}

const QString& KWalletManagerWidget::activeWalletName() const
{
    return qobject_cast<KWalletManagerWidgetItem*>(currentPage())->walletName();
}

void KWalletManagerWidget::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->provides("application/x-kwallet-wallet")) {
        e->accept();
    } else {
        e->ignore();
    }
}

void KWalletManagerWidget::dragMoveEvent(QDragMoveEvent* e)
{
    qDebug("KWalletManagerWidget::dragMoveEvent");
//     KUrl dummy;
//     QListWidgetItem *dummy2;
//     if (shouldIgnoreDropEvent(e, &dummy, &dummy2)) {
//         e->ignore();
//     } else {
//         e->accept();
//     }
}

void KWalletManagerWidget::dropEvent(QDropEvent* e)
{
    qDebug("KWalletManagerWidget::dropEvent");
//     KUrl u;
//     QListWidgetItem *item;
//     if (shouldIgnoreDropEvent(e, &u, &item)) {
//         e->ignore();
//         return;
//     }
// 
//     if (!item) {
//         // Not dropped over an item thus it is a wallet
//         const QString dest = KGlobal::dirs()->saveLocation("kwallet") + u.fileName();
//         if (QFile::exists(dest)) {
//             KMessageBox::sorry(viewport(), i18n("That wallet file already exists.  You cannot overwrite wallets."));
//             e->ignore();
//             return;
//         }
// 
//         // FIXME: verify that it is a real wallet file first
//         KIO::NetAccess::file_copy(u, KUrl(dest));
//         e->accept();
//     } else {
//         // Dropped over an item thus it is a folder
//         KWalletItem *kwi = dynamic_cast<KWalletItem *>(item);
//         Q_ASSERT(kwi);
//         if (kwi) {
//             kwi->processDropEvent(e);
//         }
//     }
}

bool KWalletManagerWidget::shouldIgnoreDropEvent(const QDropEvent* e, KUrl* u) const
{
    return false;
//     if (e->source() == viewport()) {
//         return true;
//     }
// 
//     if (!e->provides("application/x-kwallet-folder") &&
//         !e->provides("application/x-kwallet-wallet") &&
//         !e->provides("text/uri-list")) {
//         return true;
//     }
// 
//     // Over wallets folders, over nothing wallets
//     *item = itemAt(e->pos());
//     const QByteArray edata = e->encodedData(item ? "application/x-kwallet-folder" : "application/x-kwallet-wallet");
//     *u = decodeUrl(edata);
//     if (*u == KUrl()) {
//         *u = decodeUrl(e->encodedData("text/uri-list"));
//     }
// 
//     return *u == KUrl();
}

#include "kwalletmanagerwidget.moc"
