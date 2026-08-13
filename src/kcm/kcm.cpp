/*
 *  SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kcm.h"

#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QProcess>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KWallet>

#define KWALLETMANAGERINTERFACE "org.kde.KWallet"

K_PLUGIN_CLASS_WITH_JSON(KCMWallet, "kcm_kwallet.json")

KCMWallet::KCMWallet(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
    , m_data(new KWalletData(this))
{
    setAuthActionName(QStringLiteral("org.kde.kcontrol.kcmkwallet5.save"));

    // FirstUse is usually non-default, and that breaks the defaultness
    // calculation. Since we don't use it here delete it from the skeleton
    m_data->settings()->removeItem(QStringLiteral("FirstUse"));

    m_walletList = KWallet::Wallet::walletList();
}

KWalletSettings *KCMWallet::settings() const
{
    return m_data->settings();
}

QStringList KCMWallet::walletList() const
{
    return m_walletList;
}

void KCMWallet::save()
{
    QVariantMap args;
    KAuth::Action action(QLatin1String("org.kde.kcontrol.kcmkwallet5.save"));
    action.setHelperId(QStringLiteral("org.kde.kcontrol.kcmkwallet5"));

    if (!action.isValid()) {
        Q_EMIT error(i18n("Failed to create auth action"));
        return;
    }
    action.setArguments(args);

    KAuth::ExecuteJob *j = action.execute();

    if (!j->exec()) {
        if (j->error() == KAuth::ActionReply::AuthorizationDeniedError) {
            Q_EMIT error(i18n("Failed to save settings: Permission denied"));
        } else {
            Q_EMIT error(i18n("Error while authenticating action:\n%1", j->errorString()));
        }
        load();
        return;
    }

    KQuickManagedConfigModule::save();

    // this restarts kwalletd if necessary
    QDBusInterface kwalletd(QStringLiteral("org.kde.kwalletd5"), QStringLiteral("/modules/kwalletd5"), QStringLiteral(KWALLETMANAGERINTERFACE));
    // if wallet was deactivated, then kwalletd will exit upon start so check
    // the status before invoking reconfigure
    if (kwalletd.isValid()) {
        // this will eventually make kwalletd exit upon deactivation
        kwalletd.call(QStringLiteral("reconfigure"));
    }
}

void KCMWallet::launchManager()
{
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.kwalletmanager5"))) {
        QProcess::startDetached(QStringLiteral("kwalletmanager5"), QStringList(QStringLiteral("--show")));
    } else {
        QDBusInterface kwalletd(QStringLiteral("org.kde.kwalletmanager5"), QStringLiteral("/kwalletmanager5/MainWindow_1"));
        kwalletd.call(QStringLiteral("show"));
        kwalletd.call(QStringLiteral("raise"));
    }
}

void KCMWallet::createWallet(const QString &name)
{
    const QString sanitizedName = name.trimmed();

    if (sanitizedName.isEmpty()) {
        return;
    }

    std::unique_ptr<KWallet::Wallet> w(KWallet::Wallet::openWallet(sanitizedName, 0));

    if (!w) {
        Q_EMIT error(i18n("Failed to create wallet"));
    }

    m_walletList = KWallet::Wallet::walletList();
    Q_EMIT walletListChanged();
}

#include "kcm.moc"
