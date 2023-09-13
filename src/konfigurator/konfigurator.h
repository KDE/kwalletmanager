/*
    SPDX-FileCopyrightText: 2003 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _KWALLETKONFIGURATOR_H
#define _KWALLETKONFIGURATOR_H
#include "kcmutils_version.h"
#include <KCModule>
#include <KSharedConfig>
#include "ui_walletconfigwidget.h"
#include <KAuth/Action>

class WalletConfigWidget : public QWidget, public Ui::WalletConfigWidget
{
public:
    WalletConfigWidget(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
    }
};

class KWalletConfig : public KCModule
{
    Q_OBJECT
public:
    explicit KWalletConfig(QObject *parent, const KPluginMetaData &data);
    ~KWalletConfig() override;

    void load() override;
    void save() override;
    void defaults() override;

public Q_SLOTS:
    void configChanged();
    void launchManager();
    void newLocalWallet();
    void newNetworkWallet();
    void updateWalletLists();
    void deleteEntry();
    void customContextMenuRequested(const QPoint &pos);

private:
    QString newWallet();
    WalletConfigWidget *const _wcw;
    KSharedConfig::Ptr _cfg;
};

#endif
