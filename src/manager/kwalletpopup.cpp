/*
    SPDX-FileCopyrightText: 2003 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwalletpopup.h"

#include <KStandardAction>
#include <KActionCollection>
#include <QAction>
#include <KLocalizedString>
#include <KMessageBox>
#include <KWallet>


KWalletPopup::KWalletPopup(const QString &wallet, QWidget *parent, const QString &name)
    : QMenu(parent), _walletName(wallet)
{
    addSection(wallet);
    setObjectName(name);
    auto ac = new KActionCollection(this/*, "kwallet context actions"*/);
    ac->setObjectName(QStringLiteral("kwallet context actions"));
    QAction *act;

    act = ac->addAction(QStringLiteral("wallet_create"));
    act->setText(i18n("&New Wallet..."));
    connect(act, &QAction::triggered, this, &KWalletPopup::createWallet);
    addAction(act);

    act = ac->addAction(QStringLiteral("wallet-open"));
    act->setText(i18n("&Open..."));
    connect(act, &QAction::triggered, this, &KWalletPopup::openWallet);
    act->setShortcut(QKeySequence(Qt::Key_Return));
    addAction(act);

    act = ac->addAction(QStringLiteral("wallet_password"));
    act->setText(i18n("Change &Password..."));
    connect(act, &QAction::triggered, this, &KWalletPopup::changeWalletPassword);
    addAction(act);

    const QStringList ul = KWallet::Wallet::users(wallet);
    if (!ul.isEmpty()) {
        auto pm = new QMenu(this);
        pm->setObjectName(QStringLiteral("Disconnect Apps"));
        int id = 7000;
        for (QStringList::const_iterator it = ul.begin(), end(ul.end()); it != end; ++it) {
            QAction *a = pm->addAction(*it, this, &KWalletPopup::disconnectApp);
            a->setData(*it);
            id++;
        }
        QAction *act = addMenu(pm);
        act->setText(i18n("Disconnec&t"));
    }

    act = KStandardAction::close(this,
                                 SLOT(closeWallet()), ac);
    ac->addAction(QStringLiteral("wallet_close"), act);
    // FIXME: let's track this inside the manager so we don't need a dcop
    //        roundtrip here.
    act->setEnabled(KWallet::Wallet::isOpen(wallet));
    addAction(act);

    act = ac->addAction(QStringLiteral("wallet_delete"));
    act->setText(i18n("&Delete"));

    connect(act, &QAction::triggered, this, &KWalletPopup::deleteWallet);
    act->setShortcut(QKeySequence(Qt::Key_Delete));
    addAction(act);
}

KWalletPopup::~KWalletPopup()
{
}

void KWalletPopup::openWallet()
{
    Q_EMIT walletOpened(_walletName);
}

void KWalletPopup::deleteWallet()
{
    Q_EMIT walletDeleted(_walletName);
}

void KWalletPopup::closeWallet()
{
    Q_EMIT walletClosed(_walletName);
}

void KWalletPopup::changeWalletPassword()
{
    Q_EMIT walletChangePassword(_walletName);
}

void KWalletPopup::createWallet()
{
    Q_EMIT walletCreated();
}

void KWalletPopup::disconnectApp()
{
    auto a = qobject_cast<QAction *>(sender());
    Q_ASSERT(a);
    if (a)     {
        KWallet::Wallet::disconnectApplication(_walletName, a->data().toString());
    }
}



