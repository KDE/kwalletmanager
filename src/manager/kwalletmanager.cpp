/*
    SPDX-FileCopyrightText: 2003, 2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwalletmanager.h"
#include "allyourbase.h"
#include "kwallet_interface.h"
#include "kwalleteditor.h"
#include "kwalletmanager_debug.h"
#include "kwalletmanagerwidget.h"
#include "kwalletpopup.h"
#include "registercreateactionmethod.h"

#include <KActionCollection>
#include <KConfig>
#include <KConfigGroup>
#include <KIO/CommandLauncherJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardAction>
#include <KStatusNotifierItem>
#include <KTar>
#include <KWallet>
#include <KWindowSystem>
#include <KXMLGUIFactory>

#include <QAction>
#include <QCommandLineParser>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QLineEdit>
#include <QMimeDatabase>
#include <QMimeType>
#include <QTimer>

KWalletManager::KWalletManager(QCommandLineParser *commandLineParser)
    : KXmlGuiWindow()
    , _commandLineParser(commandLineParser)
{
    _kwalletdLaunch = false;
    _shuttingDown = false;
    m_kwalletdModule = nullptr;
    RegisterCreateActionsMethod::createActions(actionCollection());

    QTimer::singleShot(0, this, &KWalletManager::beginConfiguration);

    processParsedCommandLine();
}

void KWalletManager::beginConfiguration()
{
    KConfig cfg(QStringLiteral("kwalletrc")); // not sure why this setting isn't in kwalletmanagerrc...
    KConfigGroup walletConfigGroup(&cfg, QStringLiteral("Wallet"));
    if (walletConfigGroup.readEntry("Enabled", true)) {
        QTimer::singleShot(0, this, &KWalletManager::configUI);
    } else {
        int rc = KMessageBox::warningTwoActions(this,
                                                i18n("The KDE Wallet system is not enabled. Do you want me to enable it? If not, the KWalletManager will quit "
                                                     "as it cannot work without reading the wallets."),
                                                {},
                                                KStandardGuiItem::ok(),
                                                KStandardGuiItem::cancel());
        if (rc == KMessageBox::ButtonCode::PrimaryAction) {
            walletConfigGroup.writeEntry("Enabled", true);
            QTimer::singleShot(0, this, &KWalletManager::configUI);
        } else {
            QApplication::quit();
        }
    }
}

void KWalletManager::configUI()
{
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/KWalletManager"), this, QDBusConnection::ExportScriptableSlots);
    KConfig cfg(QStringLiteral("kwalletrc")); // not sure why this setting isn't in kwalletmanagerrc...
    KConfigGroup walletConfigGroup(&cfg, QStringLiteral("Wallet"));
    if (walletConfigGroup.readEntry("Launch Manager", false)) {
        _tray = new KStatusNotifierItem(this);
        _tray->setObjectName(QStringLiteral("kwalletmanager tray"));
        _tray->setCategory(KStatusNotifierItem::SystemServices);
        _tray->setStatus(KStatusNotifierItem::Passive);
        _tray->setIconByName(QStringLiteral("wallet-closed"));
        _tray->setToolTip(QStringLiteral("wallet-closed"), i18n("Wallet"), i18n("No wallets open."));
        // connect(_tray, SIGNAL(quitSelected()), SLOT(shuttingDown()));
        const QStringList wl = KWallet::Wallet::walletList();
        bool isOpen = false;
        for (QStringList::ConstIterator it = wl.begin(), end = wl.end(); it != end; ++it) {
            if (KWallet::Wallet::isOpen(*it)) {
                _tray->setIconByName(QStringLiteral("wallet-open"));
                _tray->setToolTip(QStringLiteral("wallet-open"), i18n("Wallet"), i18n("A wallet is open."));
                isOpen = true;
                break;
            }
        }
        if (!isOpen && qApp->isSessionRestored()) {
            delete _tray;
            _tray = nullptr;
            QTimer::singleShot(0, qApp, SLOT(quit()));
            return;
        }
    } else {
        _tray = nullptr;
    }

    _managerWidget = new KWalletManagerWidget(this);

    updateWalletDisplay();
    setCentralWidget(_managerWidget);
    setAutoSaveSettings(QStringLiteral("MainWindow"), true);
    QFontMetrics fm = fontMetrics();
    _managerWidget->setMinimumSize(16 * fm.height(), 18 * fm.height());

    m_kwalletdModule = new org::kde::KWallet(QStringLiteral("org.kde.kwalletd5"), QStringLiteral("/modules/kwalletd5"), QDBusConnection::sessionBus());
    connect(QDBusConnection::sessionBus().interface(), &QDBusConnectionInterface::serviceOwnerChanged, this, &KWalletManager::possiblyRescan);
    connect(QDBusConnection::sessionBus().interface(),
            &QDBusConnectionInterface::callWithCallbackFailed,
            this,
            [this](const QDBusError error, const QDBusMessage call) {
                if (call.interface() == QStringLiteral("org.kde.KWallet")) {
                    _managerWidget->setErrorMessage(error.message());
                }
            });
    connect(m_kwalletdModule, &OrgKdeKWalletInterface::allWalletsClosed, this, &KWalletManager::allWalletsClosed);
    connect(m_kwalletdModule, SIGNAL(walletClosed(QString)), this, SLOT(updateWalletDisplay()));
    connect(m_kwalletdModule, &OrgKdeKWalletInterface::walletOpened, this, &KWalletManager::aWalletWasOpened);
    connect(m_kwalletdModule, &OrgKdeKWalletInterface::walletDeleted, this, &KWalletManager::updateWalletDisplay);
    connect(m_kwalletdModule, &OrgKdeKWalletInterface::walletListDirty, this, &KWalletManager::updateWalletDisplay);
    connect(m_kwalletdModule, &OrgKdeKWalletInterface::walletCreated, this, &KWalletManager::walletCreated);
    // FIXME: slight race - a wallet can open, then we get launched, but the
    //        wallet closes before we are done opening.  We will then stay
    //        open.  Must check that a wallet is still open here.

    QAction *action = actionCollection()->addAction(QStringLiteral("wallet_create"));
    action->setText(i18n("&New Wallet..."));
    action->setIcon(QIcon::fromTheme(QStringLiteral("kwalletmanager")));
    connect(action, &QAction::triggered, this, &KWalletManager::createWallet);

    action = actionCollection()->addAction(QStringLiteral("wallet_delete"));
    action->setText(i18n("&Delete Wallet..."));
    action->setIcon(QIcon::fromTheme(QStringLiteral("trash-empty")));
    connect(action, &QAction::triggered, this, &KWalletManager::deleteWallet);

    _walletsExportAction = actionCollection()->addAction(QStringLiteral("wallet_export_encrypted"));
    _walletsExportAction->setText(i18n("Export as encrypted"));
    _walletsExportAction->setIcon(QIcon::fromTheme(QStringLiteral("document-export")));
    connect(_walletsExportAction, &QAction::triggered, this, &KWalletManager::exportWallets);

    action = actionCollection()->addAction(QStringLiteral("wallet_import_encrypted"));
    action->setText(i18n("&Import encrypted"));
    action->setIcon(QIcon::fromTheme(QStringLiteral("document-import")));
    connect(action, &QAction::triggered, this, &KWalletManager::importWallets);

    QAction *act = actionCollection()->addAction(QStringLiteral("wallet_settings"));
    act->setText(i18n("Configure &Wallet..."));
    act->setIcon(QIcon::fromTheme(QStringLiteral("configure")));

    connect(act, &QAction::triggered, this, &KWalletManager::setupWallet);
    if (_tray) {
        _tray->contextMenu()->addAction(act);
    }
    act = actionCollection()->addAction(QStringLiteral("close_all_wallets"));
    act->setText(i18n("Close &All Wallets"));
    connect(act, &QAction::triggered, this, &KWalletManager::closeAllWallets);
    if (_tray) {
        _tray->contextMenu()->addAction(act);
    }
    KStandardAction::quit(this, SLOT(shuttingDown()), actionCollection());

    KStandardAction::keyBindings(guiFactory(), &KXMLGUIFactory::showConfigureShortcutsDialog, actionCollection());

    setupGUI(Keys | Save | Create, QStringLiteral("kwalletmanager.rc"));
    setStandardToolBarMenuEnabled(false);

    if (_tray) {
        // _tray->show();
    } else {
        show();
    }

    _walletsExportAction->setDisabled(KWallet::Wallet::walletList().isEmpty());
    qApp->setObjectName(QStringLiteral("kwallet")); // hack to fix docs
}

KWalletManager::~KWalletManager()
{
    _tray = nullptr;
    delete m_kwalletdModule;
    m_kwalletdModule = nullptr;
}

void KWalletManager::kwalletdLaunch()
{
    _kwalletdLaunch = true;
}

bool KWalletManager::queryClose()
{
    if (hasUnsavedChanges() && !canIgnoreUnsavedChanges()) {
        return false;
    }

    if (!_shuttingDown) {
        if (!_tray) {
            qApp->quit();
        } else {
            _tray->hideAssociatedWindow();
        }
        return false;
    }
    return true;
}

void KWalletManager::aWalletWasOpened()
{
    if (_tray) {
        _tray->setIconByName(QStringLiteral("wallet-open"));
        _tray->setToolTip(QStringLiteral("wallet-open"), i18n("Wallet"), i18n("A wallet is open."));
        _tray->setStatus(KStatusNotifierItem::Active);
    }
    updateWalletDisplay();
    createGUI(QStringLiteral("kwalletmanager.rc"));
}

void KWalletManager::updateWalletDisplay()
{
    if (_walletsExportAction) {
        _walletsExportAction->setDisabled(KWallet::Wallet::walletList().isEmpty());
    }
    _managerWidget->updateWalletDisplay();
}

void KWalletManager::walletCreated(const QString &newWalletName)
{
    _managerWidget->updateWalletDisplay(newWalletName);
}

void KWalletManager::contextMenu(const QPoint &)
{
}

void KWalletManager::closeWallet(const QString &walletName)
{
    if (hasUnsavedChanges(walletName) && !canIgnoreUnsavedChanges()) {
        return;
    }

    int rc = KWallet::Wallet::closeWallet(walletName, false);
    if (rc != 0) {
        rc =
            KMessageBox::warningTwoActions(this,
                                           i18n("Unable to close wallet cleanly. It is probably in use by other applications. Do you wish to force it closed?"),
                                           QString(),
                                           KGuiItem(i18n("Force Closure")),
                                           KGuiItem(i18n("Do Not Force")));
        if (rc == KMessageBox::ButtonCode::PrimaryAction) {
            rc = KWallet::Wallet::closeWallet(walletName, true);
            if (rc != 0) {
                KMessageBox::error(this, i18n("Unable to force the wallet closed. Error code was %1.", rc));
            }
        }
    }

    updateWalletDisplay();
}

void KWalletManager::changeWalletPassword(const QString &walletName)
{
    KWallet::Wallet::changePassword(walletName, effectiveWinId());
}

void KWalletManager::openWalletFile(const QString &path)
{
    if (!_managerWidget->openWalletFile(path)) {
        KMessageBox::error(this, i18n("Error opening wallet %1.", path));
    }
}

void KWalletManager::allWalletsClosed()
{
    if (_tray) {
        _tray->setIconByName(QStringLiteral("wallet-closed"));
        _tray->setToolTip(QStringLiteral("wallet-closed"), i18n("Wallet"), i18n("No wallets open."));
        _tray->setStatus(KStatusNotifierItem::Passive);
    }
    possiblyQuit();
}

void KWalletManager::possiblyQuit()
{
    KConfig _cfg(QStringLiteral("kwalletrc"));
    KConfigGroup cfg(&_cfg, QStringLiteral("Wallet"));
    if (_windows.isEmpty() && !isVisible() && !cfg.readEntry("Leave Manager Open", false) && _kwalletdLaunch) {
        qApp->quit();
    }
}

void KWalletManager::editorClosed(KXmlGuiWindow *e)
{
    _windows.removeAll(e);
}

void KWalletManager::possiblyRescan(const QString &app, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner);
    Q_UNUSED(newOwner);
    if (app == QLatin1String("org.kde.kwalletd5")) {
        updateWalletDisplay();
    }
}

void KWalletManager::createWallet()
{
    QString txt = i18n("Please choose a name for the new wallet:");

    if (!KWallet::Wallet::isEnabled()) {
        // FIXME: KMessageBox::warningYesNo(this, i1_8n("KWallet is not enabled.  Do you want to enable it?"), QString(), i18n("Enable"), i18n("Keep
        // Disabled"));
        return;
    }
    // TODO port to QRegularExpressionValidator
    QDialog nameDialog(this);
    nameDialog.setWindowTitle(i18nc("@title:window", "New Wallet"));
    nameDialog.setLayout(new QVBoxLayout);
    nameDialog.layout()->addWidget(new QLabel(txt));
    auto lineEdit = new QLineEdit;
    nameDialog.layout()->addWidget(lineEdit);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &nameDialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &nameDialog, &QDialog::reject);
    nameDialog.layout()->addWidget(buttonBox);

    QString name;
    do {
        if (!nameDialog.exec()) {
            return;
        }

        name = lineEdit->text();
        if (name.trimmed().isEmpty()) {
            KMessageBox::error(this, i18n("Empty name is not supported. Please select a new one."), i18n("Create new wallet"));
            return;
        }

        if (_managerWidget->hasWallet(name)) {
            int rc = KMessageBox::questionTwoActions(this,
                                                     i18n("Sorry, that wallet already exists. Try a new name?"),
                                                     QString(),
                                                     KGuiItem(i18n("Try New")),
                                                     KGuiItem(i18n("Do Not Try")));
            if (rc == KMessageBox::ButtonCode::SecondaryAction) {
                return;
            }
            lineEdit->clear();
        } else {
            break;
        }
    } while (true);

    // Small race here - the wallet could be created on us already.
    if (!name.isEmpty()) {
        // attempt open the wallet to create it, then dispose it
        // as it'll appear in on the main window via the walletCreated signal
        // emmitted by the kwalletd
        KWallet::Wallet::openWallet(name, effectiveWinId());
    }
}

void KWalletManager::deleteWallet()
{
    QString walletName = _managerWidget->activeWalletName();
    if (walletName.isEmpty()) {
        return;
    }
    int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the wallet '%1'?", walletName), QString(), KStandardGuiItem::del());
    if (rc != KMessageBox::Continue) {
        return;
    }
    rc = KWallet::Wallet::deleteWallet(walletName);
    if (rc != 0) {
        KMessageBox::error(this, i18n("Unable to delete the wallet. Error code was %1.", rc));
    }
}

void KWalletManager::openWallet(const QString &walletName)
{
    _managerWidget->openWallet(walletName);
}

void KWalletManager::shuttingDown()
{
    if (hasUnsavedChanges() && !canIgnoreUnsavedChanges()) {
        return;
    }

    _shuttingDown = true;
    qApp->quit();
}

void KWalletManager::setupWallet()
{
    auto job = new KIO::CommandLauncherJob(QStringLiteral("kcmshell6"), {QStringLiteral("kcm_kwallet5")});
    job->start();
}

void KWalletManager::closeAllWallets()
{
    if (hasUnsavedChanges() && !canIgnoreUnsavedChanges()) {
        return;
    }

    m_kwalletdModule->closeAllWallets();
}

void KWalletManager::exportWallets()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kwalletd/");
    const QDir dir(path);
    dir.mkpath(path);

    Q_ASSERT(dir.exists());

    const QStringList filesList =
        dir.entryList(QStringList() << QStringLiteral("*.kwl") << QStringLiteral("*.salt"), QDir::Files | QDir::Readable | QDir::NoSymLinks);

    Q_ASSERT(!filesList.isEmpty());

    const QString destination = QFileDialog::getSaveFileName(this, i18n("File name"));
    if (destination.isEmpty()) {
        return;
    }

    KTar archive(destination);
    if (!archive.open(QIODevice::WriteOnly)) {
        KMessageBox::error(this, i18n("Failed to open file for writing"));
        return;
    }

    for (int i = 0; i < filesList.size(); i++) {
        archive.addLocalFile(path + filesList.at(i), filesList.at(i));
    }
}

void KWalletManager::importWallets()
{
    const QString source = QFileDialog::getOpenFileName(this, i18n("Select file"));
    const QString destinationDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kwalletd/");
    QDir().mkpath(destinationDir);

    if (source.isEmpty()) {
        return;
    }

    KTar archive(source);
    if (!archive.open(QIODevice::ReadOnly)) {
        KMessageBox::error(this, i18n("Failed to open file"));
        return;
    }

    const KArchiveDirectory *archiveDir = archive.directory();
    const QStringList archiveEntries = archiveDir->entries();

    for (int i = 0; i < archiveEntries.size(); i++) {
        if (QFile::exists(destinationDir + archiveEntries.at(i)) && archiveEntries.at(i).endsWith(QLatin1String(".kwl"))) {
            QString walletName = archiveEntries.at(i);
            // remove ".kwl"
            walletName.chop(4);
            KMessageBox::error(this, i18n("Wallet named %1 already exists, Operation aborted", walletName));
            return;
        }
    }

    if (!archiveDir->copyTo(destinationDir, false)) {
        KMessageBox::error(this, i18n("Failed to copy files"));
        return;
    }
}

bool KWalletManager::hasUnsavedChanges(const QString &name) const
{
    return (_managerWidget ? _managerWidget->hasUnsavedChanges(name) : false);
}

bool KWalletManager::canIgnoreUnsavedChanges()
{
    int rc = KMessageBox::warningTwoActions(this, i18n("Ignore unsaved changes?"), {}, KGuiItem(i18n("Ignore")), KStandardGuiItem::cancel());
    return (rc == KMessageBox::ButtonCode::PrimaryAction);
}

void KWalletManager::handleActivate(const QStringList &arguments, const QString &workingDirectory)
{
    Q_UNUSED(workingDirectory);

    // KDBusService::Unique handling
    _commandLineParser->parse(arguments);

    processParsedCommandLine();
}

void KWalletManager::processParsedCommandLine()
{
    if (_commandLineParser->isSet(QStringLiteral("kwalletd"))) {
        kwalletdLaunch();
    } else {
        activateForStartLikeCall();

        const QStringList positionalArguments = _commandLineParser->positionalArguments();
        QStringList localFiles;
        for (const QString &arg : positionalArguments) {
            const QString fn = QFileInfo(arg).absoluteFilePath();
            if (QFile::exists(fn)) {
                localFiles.append(fn);
            } else {
                openWallet(arg);
            }
        }
        tryOpenWalletFiles(localFiles);
    }
}

void KWalletManager::handleOpen(const QList<QUrl> &urls)
{
    QStringList localFiles;
    for (const QUrl &url : urls) {
        localFiles.append(url.toLocalFile());
    }
    tryOpenWalletFiles(localFiles);
    activateForStartLikeCall();
}

void KWalletManager::tryOpenWalletFiles(const QStringList &localFiles)
{
    QMimeDatabase mimeDb;
    for (const QString &localFile : localFiles) {
        if (QFile::exists(localFile)) {
            QMimeType mt = mimeDb.mimeTypeForFile(localFile, QMimeDatabase::MatchContent);

            if (mt.isValid() && mt.inherits(QStringLiteral("application/x-kwallet"))) {
                openWalletFile(localFile);
            }
        }
    }
}

void KWalletManager::activateForStartLikeCall()
{
    show();
    KWindowSystem::updateStartupId(windowHandle());

    if (isMinimized())
        setWindowState(windowState() & ~Qt::WindowMinimized);
    else if (_tray && !isVisible()) // TODO: how does this relate to show() above?
        _tray->activate(QPoint());

    // if there is a window visible, now or before, always activate it
    if (isVisible()) {
        if (!isActiveWindow()) {
            raise();
            KWindowSystem::activateWindow(windowHandle());
        }
    }
}

#include "moc_kwalletmanager.cpp"
