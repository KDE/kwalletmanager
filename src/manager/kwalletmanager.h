/*
    SPDX-FileCopyrightText: 2003, 2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWALLETMANAGER_H
#define KWALLETMANAGER_H
#include <QObject>
#include <kxmlguiwindow.h>

class KWalletManagerWidget;
class KStatusNotifierItem;
class QListWidgetItem;
class OrgKdeKWalletInterface;
class QAction;

class KWalletManager : public KXmlGuiWindow
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.kde.kwallet.kwalletmanager")

public:
    explicit KWalletManager(QWidget *parent = nullptr, const QString &name = QString(), Qt::WindowFlags f = {});
    ~KWalletManager() override;

    void kwalletdLaunch();
    bool hasUnsavedChanges(const QString& name = QString()) const;
    bool canIgnoreUnsavedChanges();

public Q_SLOTS:
    void createWallet();
    void deleteWallet();
    void closeWallet(const QString &walletName);
    void changeWalletPassword(const QString &walletName);
    void openWallet(const QString &walletName);
    void openWalletFile(const QString &path);
//         void openWallet(QListWidgetItem *item);
    void contextMenu(const QPoint &pos);
    void walletCreated(const QString &walletName);

protected:
    bool queryClose() override;

private:
public Q_SLOTS: //dbus
    Q_SCRIPTABLE void allWalletsClosed();
    Q_SCRIPTABLE void updateWalletDisplay();
    Q_SCRIPTABLE void aWalletWasOpened();

private Q_SLOTS:
    void shuttingDown();
    void possiblyQuit();
    void editorClosed(KXmlGuiWindow *e);
    void possiblyRescan(const QString &app, const QString &, const QString &);
    void setupWallet();
    void closeAllWallets();
    void exportWallets();
    void importWallets();
    void beginConfiguration();
    void configUI();

private:
    KStatusNotifierItem *_tray;
    bool _shuttingDown;
    KWalletManagerWidget *_managerWidget;
    OrgKdeKWalletInterface *m_kwalletdModule;
    QList<KXmlGuiWindow *> _windows;
    bool _kwalletdLaunch;
    QAction *_walletsExportAction = nullptr;
};

#endif
