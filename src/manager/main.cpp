/*
    SPDX-FileCopyrightText: 2003-2005 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "../kwalletmanager_version.h"
#include "kwalletmanager.h"

#include <KDBusAddons/kdbusservice.h>
#include <KLocalizedString>
#include <KAboutData>

#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <KCrash>

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    QApplication a(argc, argv);
    KCrash::initialize();
    KLocalizedString::setApplicationDomain("kwalletmanager");

    a.setWindowIcon(QIcon::fromTheme(QStringLiteral("kwalletmanager")));

    KAboutData aboutData(QStringLiteral("kwalletmanager5"),
                 i18n("Wallet Manager"),
                 QStringLiteral(KWALLETMANAGER_VERSION_STRING),
                 i18n("KDE Wallet Management Tool"),
                 KAboutLicense::GPL,
                 i18n("Copyright ©2013–2017, KWallet Manager authors"),
                 QString(),
                 QStringLiteral("https://utils.kde.org/projects/kwalletmanager/"));

    aboutData.addAuthor(i18n("Valentin Rusu"),
                        i18n("Maintainer, user interface refactoring"),
                        QStringLiteral("kde@rusu.info"));
    aboutData.addAuthor(i18n("George Staikos"),
                        i18n("Original author and former maintainer"),
                        QStringLiteral("staikos@kde.org"));
    aboutData.addAuthor(i18n("Michael Leupold"),
                        i18n("Developer and former maintainer"),
                        QStringLiteral("lemma@confuego.org"));
    aboutData.addAuthor(i18n("Isaac Clerencia"),
                        i18n("Developer"),
                        QStringLiteral("isaac@warp.es"));

    aboutData.setOrganizationDomain("kde.org");

    KAboutData::setApplicationData(aboutData);

    KDBusService dbssvc(KDBusService::Unique);

    QCommandLineParser parser;

    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringLiteral("show"), i18n("Show window on startup")));
    parser.addOption(QCommandLineOption(QStringLiteral("kwalletd"), i18n("For use by kwalletd only")));
    parser.addOption(QCommandLineOption(QStringLiteral("name"), i18n("A wallet name")));
    parser.process(a);
    aboutData.processCommandLine(&parser);
    KWalletManager wm;
    QObject::connect(&dbssvc, &KDBusService::activateRequested, &wm, &QWidget::activateWindow);

    if (parser.isSet(QStringLiteral("show"))) {
        wm.show();
    }

    if (parser.isSet(QStringLiteral("kwalletd"))) {
        wm.kwalletdLaunch();
    }

    const QStringList arguments = parser.positionalArguments();
    for (int i = 1; i < arguments.count(); ++i) {
        QString fn = QFileInfo(arguments.at(i)).absoluteFilePath();
        if (QFile::exists(fn)) {
            QMimeDatabase mimeDb;
            QMimeType mt = mimeDb.mimeTypeForFile(fn, QMimeDatabase::MatchContent);

            if (mt.isValid() && mt.inherits(QStringLiteral("application/x-kwallet"))) {
                wm.openWalletFile(fn);
            }
        } else {
            wm.openWallet(arguments.at(i));
        }
    }

    return a.exec();
}

