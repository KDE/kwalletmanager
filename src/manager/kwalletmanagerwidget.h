/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KWALLETMANAGERWIDGET_H
#define KWALLETMANAGERWIDGET_H

#include <KPageWidget>

class QUrl;
class QDropEvent;
class KWalletManagerWidgetItem;
class QDragEnterEvent;
class QDragMoveEvent;

class KWalletManagerWidget : public KPageWidget
{
    Q_OBJECT
public:
    explicit KWalletManagerWidget(QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~KWalletManagerWidget() override;

    void updateWalletDisplay(const QString &selectWallet = QString());
    bool hasWallet(const QString &) const;
    bool openWalletFile(const QString &path);
    bool openWallet(const QString &name);

    QString activeWalletName() const;
    bool hasUnsavedChanges(const QString& name) const;

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;

private Q_SLOTS:
    void onCurrentPageChanged(KPageWidgetItem *, KPageWidgetItem *);

private:
    bool shouldIgnoreDropEvent(const QDropEvent *e, QUrl *u) const;

    using WalletPagesHash = QHash<QString, KWalletManagerWidgetItem *>;
    WalletPagesHash _walletPages;
};

#endif // KWALLETMANAGERWIDGET_H

