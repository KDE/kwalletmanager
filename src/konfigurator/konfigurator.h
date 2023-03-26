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
#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 240, 0)
#include <KAuth/Action>
#endif

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
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    explicit KWalletConfig(QWidget *parent = nullptr, const QVariantList & = QVariantList());
#else
    explicit KWalletConfig(QObject *parent, const KPluginMetaData &data, const QVariantList & = QVariantList());
#endif
    ~KWalletConfig() override;

    void load() override;
    void save() override;
    void defaults() override;
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    QString quickHelp() const override;
#endif

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
#if KCMUTILS_VERSION >= QT_VERSION_CHECK(5, 240, 0)
    KAuth::Action m_authAction;
#endif
};

#endif
