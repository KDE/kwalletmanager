/*
 *  SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <KQuickManagedConfigModule>

#include "kwalletdata.h"
#include "kwalletsettings.h"

class KCMWallet : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(KWalletSettings *settings READ settings CONSTANT)
    Q_PROPERTY(QStringList walletList READ walletList NOTIFY walletListChanged)

public:
    KCMWallet(QObject *parent, const KPluginMetaData &metaData);

    KWalletSettings *settings() const;

    QStringList walletList() const;
    Q_SIGNAL void walletListChanged();

    Q_INVOKABLE void launchManager();
    Q_INVOKABLE void createWallet(const QString &name);

    Q_SIGNAL void error(const QString &message);

    void save() override;

private:
    KWalletData *m_data;
    QStringList m_walletList;
};
