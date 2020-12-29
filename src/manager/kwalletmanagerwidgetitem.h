/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KWALLETMANAGERWIDGETITEM_H
#define KWALLETMANAGERWIDGETITEM_H

#include <KPageWidgetModel>

class WalletControlWidget;
class KWalletManagerWidgetItem : public KPageWidgetItem
{
    Q_OBJECT
public:
    explicit KWalletManagerWidgetItem(QWidget *widgetParent, const QString &walletName);

    void updateWalletDisplay();
    const QString &walletName() const
    {
        return _walletName;
    }
    bool openWallet();
    bool hasUnsavedChanges() const;

private:
    WalletControlWidget *_controlWidget;
    QString             _walletName;
};

#endif // KWALLETMANAGERWIDGETITEM_H
