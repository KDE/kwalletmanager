/*
   Copyright (C) 2003-2005 George Staikos <staikos@kde.org>

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

#include "../kwalletmanager_version.h"
#include "kwalletmanager.h"

#include <KDBusAddons/kdbusservice.h>
#include <klocalizedstring.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    /**
     * enable high dpi support
     */
    a.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    QCoreApplication::setApplicationName("kwalletmanager5");
    QCoreApplication::setApplicationVersion(QStringLiteral(KWALLETMANAGER_VERSION_STRING));
    QCoreApplication::setOrganizationName("KDE");
    QCoreApplication::setOrganizationDomain("kde.org");
    QApplication::setApplicationDisplayName(i18n("Wallet Manager"));

    KDBusService dbssvc(KDBusService::Unique);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption(QCommandLineOption("show", i18n("Show window on startup")));
    parser.addOption(QCommandLineOption("kwalletd", i18n("For use by kwalletd only")));
    parser.addOption(QCommandLineOption("name", i18n("A wallet name")));

    parser.process(a);
    KWalletManager wm;
    QObject::connect(&dbssvc, &KDBusService::activateRequested, &wm, &QWidget::activateWindow);

    if (parser.isSet("show")) {
        wm.show();
    }

    if (parser.isSet("kwalletd")) {
        wm.kwalletdLaunch();
    }

    const QStringList arguments = parser.positionalArguments();
    for (int i = 1; i < arguments.count(); ++i) {
        QString fn = QFileInfo(arguments.at(i)).absoluteFilePath();
        if (QFile::exists(fn)) {
            QMimeDatabase mimeDb;
            QMimeType mt = mimeDb.mimeTypeForFile(fn, QMimeDatabase::MatchContent);

            if (mt.isValid() && mt.inherits(QLatin1String("application/x-kwallet"))) {
                wm.openWalletFile(fn);
            }
        } else {
            wm.openWallet(arguments.at(i));
        }
    }

    return a.exec();
}

