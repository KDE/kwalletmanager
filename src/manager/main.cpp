/*
    SPDX-FileCopyrightText: 2003-2005 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "../kwalletmanager_version.h"
#include "kwalletmanager.h"

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedString>

#include <KCrash>
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kwalletmanager"));

    a.setWindowIcon(QIcon::fromTheme(QStringLiteral("kwalletmanager")));

    KAboutData aboutData(QStringLiteral("kwalletmanager"),
                         i18n("Wallet Manager"),
                         QStringLiteral(KWALLETMANAGER_VERSION_STRING),
                         i18n("KDE Wallet Management Tool"),
                         KAboutLicense::GPL,
                         i18n("Copyright ©2013–2017, KWallet Manager authors"),
                         QString(),
                         QStringLiteral("https://apps.kde.org/kwalletmanager5"));

    aboutData.addAuthor(i18nc("@info:credit", "Valentin Rusu"), i18n("Former Maintainer, user interface refactoring"), QStringLiteral("kde@rusu.info"));
    aboutData.addAuthor(i18nc("@info:credit", "George Staikos"), i18n("Original author and former maintainer"), QStringLiteral("staikos@kde.org"));
    aboutData.addAuthor(i18nc("@info:credit", "Michael Leupold"), i18n("Developer and former maintainer"), QStringLiteral("lemma@confuego.org"));
    aboutData.addAuthor(i18nc("@info:credit", "Isaac Clerencia"), i18n("Developer"), QStringLiteral("isaac@warp.es"));

    KAboutData::setApplicationData(aboutData);
    KCrash::initialize();
    QCommandLineParser parser;

    aboutData.setupCommandLine(&parser);
    QCommandLineOption showOption(QStringLiteral("show"), i18n("Ignored, for compatibility"));
    showOption.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(showOption);
    parser.addOption(QCommandLineOption(QStringLiteral("kwalletd"), i18n("For use by kwalletd only")));
    parser.addPositionalArgument(QStringLiteral("name"), i18n("A wallet name"));
    parser.process(a);
    aboutData.processCommandLine(&parser);

    KDBusService dbssvc(KDBusService::Unique);

    auto wm = new KWalletManager(&parser);
    QObject::connect(&dbssvc, &KDBusService::activateRequested, wm, &KWalletManager::handleActivate);
    QObject::connect(&dbssvc, &KDBusService::openRequested, wm, &KWalletManager::handleOpen);

    return a.exec();
}
