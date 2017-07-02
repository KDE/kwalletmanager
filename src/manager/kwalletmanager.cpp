/*
   Copyright (C) 2003,2004 George Staikos <staikos@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kwalletmanager.h"
#include "kwalletmanagerwidget.h"
#include "kwalletpopup.h"
#include "kwalleteditor.h"
#include "allyourbase.h"
#include "kwallet_interface.h"
#include "registercreateactionmethod.h"

#include <klocalizedstring.h>
#include <KService/KToolInvocation>

#include <qaction.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandardaction.h>
#include <kstatusnotifieritem.h>
#include <kwallet.h>
#include <kxmlguifactory.h>
#include <QIcon>
#include <kactioncollection.h>
#include <kconfiggroup.h>
#include <KTar>

#include <QDebug>
#include <QPointer>
#include <QRegExp>
#include <QRegExpValidator>
#include <QTimer>
#include <QFileDialog>
#include <QDialog>
#include <QLineEdit>


KWalletManager::KWalletManager(QWidget *parent, const char *name, Qt::WindowFlags f)
    : KXmlGuiWindow(parent, f)
{
    _kwalletdLaunch = false;
    _shuttingDown = false;
    m_kwalletdModule = 0;
    setObjectName(QLatin1String(name));
    RegisterCreateActionsMethod::createActions(actionCollection());

    QTimer::singleShot(0, this, SLOT(beginConfiguration()));
}

void KWalletManager::beginConfiguration() {
    KConfig cfg(QLatin1String("kwalletrc"));    // not sure why this setting isn't in kwalletmanagerrc...
    KConfigGroup walletConfigGroup(&cfg, "Wallet");
    if (walletConfigGroup.readEntry("Enabled", true)){
        QTimer::singleShot(0, this, SLOT(configUI()));
    } else {
        int rc = KMessageBox::warningYesNo(this,
            i18n("The KDE Wallet system is not enabled. Do you want me to enable it? If not, the KWalletManager will quit as it cannot work without reading the wallets."));
        if (rc == KMessageBox::Yes) {
            walletConfigGroup.writeEntry("Enabled", true);
            QTimer::singleShot(0, this, SLOT(configUI()));
        } else {
            QApplication::quit();
        }
    }
}

void KWalletManager::configUI() {
    QDBusConnection::sessionBus().registerObject(QLatin1String("/KWalletManager"), this, QDBusConnection::ExportScriptableSlots);
    KConfig cfg(QLatin1String("kwalletrc"));    // not sure why this setting isn't in kwalletmanagerrc...
    KConfigGroup walletConfigGroup(&cfg, "Wallet");
    if (walletConfigGroup.readEntry("Launch Manager", false)) {
        _tray = new KStatusNotifierItem(this);
        _tray->setObjectName(QLatin1String("kwalletmanager tray"));
        _tray->setCategory(KStatusNotifierItem::SystemServices);
        _tray->setStatus(KStatusNotifierItem::Passive);
        _tray->setIconByName(QLatin1String("wallet-closed"));
        _tray->setToolTip(QLatin1String("wallet-closed"), i18n("Wallet"), i18n("No wallets open."));
        //connect(_tray, SIGNAL(quitSelected()), SLOT(shuttingDown()));
        const QStringList wl = KWallet::Wallet::walletList();
        bool isOpen = false;
        for (QStringList::ConstIterator it = wl.begin(); it != wl.end(); ++it) {
            if (KWallet::Wallet::isOpen(*it)) {
                _tray->setIconByName(QLatin1String("wallet-open"));
                _tray->setToolTip(QLatin1String("wallet-open"), i18n("Wallet"), i18n("A wallet is open."));
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
    setAutoSaveSettings(QLatin1String("MainWindow"), true);
    QFontMetrics fm = fontMetrics();
    _managerWidget->setMinimumSize(16*fm.height(), 18*fm.height());

    m_kwalletdModule = new org::kde::KWallet(QLatin1String("org.kde.kwalletd5"), QLatin1String("/modules/kwalletd5"), QDBusConnection::sessionBus());
    connect(QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)), this,
            SLOT(possiblyRescan(QString,QString,QString)));
    connect(m_kwalletdModule, &OrgKdeKWalletInterface::allWalletsClosed, this, &KWalletManager::allWalletsClosed);
    connect(m_kwalletdModule, SIGNAL(walletClosed(QString)),
            this, SLOT(updateWalletDisplay()));

    connect(m_kwalletdModule, &OrgKdeKWalletInterface::walletOpened, this, &KWalletManager::aWalletWasOpened);
    connect(m_kwalletdModule, &OrgKdeKWalletInterface::walletDeleted, this, &KWalletManager::updateWalletDisplay);
    connect(m_kwalletdModule, &OrgKdeKWalletInterface::walletListDirty, this, &KWalletManager::updateWalletDisplay);
    connect(m_kwalletdModule, &OrgKdeKWalletInterface::walletCreated, this, &KWalletManager::walletCreated);
    // FIXME: slight race - a wallet can open, then we get launched, but the
    //        wallet closes before we are done opening.  We will then stay
    //        open.  Must check that a wallet is still open here.

    QAction *action = actionCollection()->addAction(QLatin1String("wallet_create"));
    action->setText(i18n("&New Wallet..."));
    action->setIcon(QIcon::fromTheme(QLatin1String("kwalletmanager")));
    connect(action, SIGNAL(triggered()), SLOT(createWallet()));

    action = actionCollection()->addAction(QLatin1String("wallet_open"));
    action->setText(i18n("Open Wallet..."));
    connect(action, SIGNAL(triggered()), this, SLOT(openWallet()));

    action = actionCollection()->addAction(QLatin1String("wallet_delete"));
    action->setText(i18n("&Delete Wallet..."));
    action->setIcon(QIcon::fromTheme(QLatin1String("trash-empty")));
    connect(action, &QAction::triggered, this, &KWalletManager::deleteWallet);

    _walletsExportAction = actionCollection()->addAction("wallet_export_encrypted");
    _walletsExportAction->setText(i18n("Export as encrypted"));
    _walletsExportAction->setIcon(QIcon::fromTheme("document-export"));
    connect(_walletsExportAction, &QAction::triggered, this, &KWalletManager::exportWallets);

    action = actionCollection()->addAction("wallet_import_encrypted");
    action->setText(i18n("&Import encrypted"));
    action->setIcon(QIcon::fromTheme("document-import"));
    connect(action, &QAction::triggered, this, &KWalletManager::importWallets);

    QAction *act = actionCollection()->addAction(QLatin1String("wallet_settings"));
    act->setText(i18n("Configure &Wallet..."));
    act->setIcon(QIcon::fromTheme(QLatin1String("configure")));

    connect(act, &QAction::triggered, this, &KWalletManager::setupWallet);
    if (_tray) {
        _tray->contextMenu()->addAction(act);
    }
    act = actionCollection()->addAction(QLatin1String("close_all_wallets"));
    act->setText(i18n("Close &All Wallets"));
    connect(act, &QAction::triggered, this, &KWalletManager::closeAllWallets);
    if (_tray) {
        _tray->contextMenu()->addAction(act);
    }
    KStandardAction::quit(this, SLOT(shuttingDown()), actionCollection());
    KStandardAction::keyBindings(guiFactory(), SLOT(configureShortcuts()),
                                 actionCollection());

    setupGUI(Keys | Save | Create, QLatin1String("kwalletmanager.rc"));
    setStandardToolBarMenuEnabled(false);

    if (_tray) {
//        _tray->show();
    } else {
        show();
    }

    _walletsExportAction->setDisabled(KWallet::Wallet::walletList().isEmpty());
    qApp->setObjectName(QLatin1String("kwallet"));   // hack to fix docs
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
            hide();
        }
        return false;
    }
    return true;
}

void KWalletManager::aWalletWasOpened()
{
    if (_tray) {
        _tray->setIconByName(QLatin1String("wallet-open"));
        _tray->setToolTip(QLatin1String("wallet-open"), i18n("Wallet"), i18n("A wallet is open."));
        _tray->setStatus(KStatusNotifierItem::Active);
    }
    updateWalletDisplay();
    createGUI(QLatin1String("kwalletmanager.rc"));
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
        rc = KMessageBox::warningYesNo(this, i18n("Unable to close wallet cleanly. It is probably in use by other applications. Do you wish to force it closed?"), QString(), KGuiItem(i18n("Force Closure")), KGuiItem(i18n("Do Not Force")));
        if (rc == KMessageBox::Yes) {
            rc = KWallet::Wallet::closeWallet(walletName, true);
            if (rc != 0) {
                KMessageBox::sorry(this, i18n("Unable to force the wallet closed. Error code was %1.", rc));
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
        KMessageBox::sorry(this, i18n("Error opening wallet %1.", path));
    }
}

void KWalletManager::allWalletsClosed()
{
    if (_tray) {
        _tray->setIconByName(QLatin1String("wallet-closed"));
        _tray->setToolTip(QLatin1String("wallet-closed"), i18n("Wallet"), i18n("No wallets open."));
        _tray->setStatus(KStatusNotifierItem::Passive);
    }
    possiblyQuit();
}

void KWalletManager::possiblyQuit()
{
    KConfig _cfg(QLatin1String("kwalletrc"));
    KConfigGroup cfg(&_cfg, "Wallet");
    if (_windows.isEmpty() &&
            !isVisible() &&
            !cfg.readEntry("Leave Manager Open", false) &&
            _kwalletdLaunch) {
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
    QRegExpValidator validator(QRegExp(QLatin1String("^[\\w\\^\\&\\'\\@\\{\\}\\[\\]\\,\\$\\=\\!\\-\\#\\(\\)\\%\\.\\+\\_\\s]+$")), this);

    if (!KWallet::Wallet::isEnabled()) {
        // FIXME: KMessageBox::warningYesNo(this, i1_8n("KWallet is not enabled.  Do you want to enable it?"), QString(), i18n("Enable"), i18n("Keep Disabled"));
        return;
    }

    QDialog nameDialog(this);
    nameDialog.setWindowTitle(i18n("New Wallet"));
    nameDialog.setLayout(new QVBoxLayout);
    nameDialog.layout()->addWidget(new QLabel(txt));
    QLineEdit *lineEdit = new QLineEdit;
    lineEdit->setValidator(&validator);
    nameDialog.layout()->addWidget(lineEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &nameDialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &nameDialog, &QDialog::reject);
    nameDialog.layout()->addWidget(buttonBox);

    QString name;
    do {
        if (!nameDialog.exec()) {
            return;
        }

        name = lineEdit->text();
        if (name.isEmpty()) {
            return;
        }

        if (_managerWidget->hasWallet(name)) {
            int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that wallet already exists. Try a new name?"), QString(), KGuiItem(i18n("Try New")), KGuiItem(i18n("Do Not Try")));
            if (rc == KMessageBox::No) {
                return;
            }
            lineEdit->clear();
        } else  {
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
        KMessageBox::sorry(this, i18n("Unable to delete the wallet. Error code was %1.", rc));
    }
}

void KWalletManager::openWallet(const QString &walletName)
{
    _managerWidget->openWallet(walletName);
}

void KWalletManager::openWallet()
{
    qWarning("TODO: implement openWallet from file");
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
    KToolInvocation::startServiceByDesktopName(QLatin1String("kwalletconfig5"));
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
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kwalletd/";
    const QDir dir(path);
    dir.mkpath(path);

    Q_ASSERT(dir.exists());

    const QStringList filesList = dir.entryList(QStringList() << "*.kwl" << "*.salt",
                                                QDir::Files | QDir::Readable | QDir::NoSymLinks);

    Q_ASSERT(!filesList.isEmpty());

    const QString destination = QFileDialog::getSaveFileName(this,i18n("File name"));
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
    const QString destinationDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kwalletd/";
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
        if (QFile::exists(destinationDir + archiveEntries.at(i))
                && archiveEntries.at(i).endsWith(".kwl")) {
            QString walletName = archiveEntries.at(i);
            // remove ".kwl"
            walletName.chop(4);
            KMessageBox::error(this, i18n("Wallet named %1 already exists, Operation aborted",
                                          walletName));
            return;
        }
    }

    if (!archiveDir->copyTo(destinationDir, false)) {
        KMessageBox::error(this,i18n("Failed to copy files"));
        return;
    }

}

bool KWalletManager::hasUnsavedChanges(const QString &name) const
{
    return _managerWidget->hasUnsavedChanges(name);
}

bool KWalletManager::canIgnoreUnsavedChanges()
{
    int rc = KMessageBox::warningYesNo(this, i18n("Ignore unsaved changes?"));
    return (rc == KMessageBox::Yes);
}
