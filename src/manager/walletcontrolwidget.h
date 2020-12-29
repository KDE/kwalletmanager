/*
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef WALLETCONTROLWIDGET_H
#define WALLETCONTROLWIDGET_H

#include "ui_walletcontrolwidget.h"

class KWalletEditor;
class ApplicationsManager;

namespace KWallet
{
class Wallet;
}

class WalletControlWidget : public QWidget, public Ui::WalletControlWidget
{
    Q_OBJECT
public:
    explicit WalletControlWidget(QWidget *parent, const QString &walletName);

    bool openWallet();
    bool hasUnsavedChanges() const;

public Q_SLOTS:
    void onSetupWidget();
    void onOpenClose();
    void onWalletClosed();
    void updateWalletDisplay();
    void onDisconnectApplication();
    void onChangePassword();

protected:
    void hideEvent(QHideEvent *) override;
    void showEvent(QShowEvent *) override;

private:
    QString             _walletName;
    KWallet::Wallet    *_wallet;
    KWalletEditor      *_walletEditor;
    ApplicationsManager *_applicationsManager;
};

#endif // WALLETCONTROLWIDGET_H
