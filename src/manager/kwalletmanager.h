/*
    SPDX-FileCopyrightText: 2003, 2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWALLETMANAGER_H
#define KWALLETMANAGER_H

#include <KXmlGuiWindow>

#include <QObject>

class KWalletManagerWidget;
class KStatusNotifierItem;
class QListWidgetItem;
class OrgKdeKWalletInterface;
class QAction;
class QCommandLineParser;

class KWalletManager : public KXmlGuiWindow
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.kde.kwallet.kwalletmanager")

public:
    explicit KWalletManager(QCommandLineParser *commandLineParser);
    ~KWalletManager() override;

    void kwalletdLaunch();
    bool hasUnsavedChanges(const QString &name = QString()) const;
    bool canIgnoreUnsavedChanges();
    void handleActivate(const QStringList &arguments, const QString &workingDirectory);
    void handleOpen(const QList<QUrl> &urls);

public Q_SLOTS:
    void createWallet();
    void deleteWallet();
    void closeWallet(const QString &walletName);
    void changeWalletPassword(const QString &walletName);
    void openWallet(const QString &walletName);
    void openWalletFile(const QString &path);
    // void openWallet(QListWidgetItem *item);
    void contextMenu(const QPoint &pos);
    void walletCreated(const QString &walletName);

protected:
    bool queryClose() override;

private:
public Q_SLOTS: // dbus
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
    void activateForStartLikeCall();
    void tryOpenWalletFiles(const QStringList &localFiles);
    void processParsedCommandLine();

private:
    QCommandLineParser *const _commandLineParser;
    KStatusNotifierItem *_tray = nullptr;
    bool _shuttingDown = false;
    KWalletManagerWidget *_managerWidget = nullptr;
    OrgKdeKWalletInterface *m_kwalletdModule = nullptr;
    QList<KXmlGuiWindow *> _windows;
    bool _kwalletdLaunch = false;
    QAction *_walletsExportAction = nullptr;
};

#endif
