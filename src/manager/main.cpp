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

#include "kwalletmanager.h"

#include <KDBusAddons/kdbusservice.h>
#include <klocalizedstring.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QEventLoopLocker>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>

class MyApp : public QApplication
{
public:
    MyApp(int &argc, char **argv) : QApplication(argc, argv)
    {
        //was: KGlobal::ref(); ported to QEeventLoopLocker
    }

    virtual ~MyApp() {}

    virtual int newInstance()
    {
        return 0;
    }
private:
    QEventLoopLocker m_locker;
};


int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName("kwalletmanager5");
    QCoreApplication::setApplicationVersion("3.0");
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

    MyApp a(argc, argv);

    KWalletManager wm;
    wm.setCaption(i18n("Wallet Manager"));

    if (parser.isSet("show")) {
        wm.show();
    }

    if (parser.isSet("kwalletd")) {
        wm.kwalletdLaunch();
    }

    for (int i = 0; i < a.arguments().count(); ++i) {
        QString fn = QFileInfo(a.arguments().at(i)).absoluteFilePath();
        if (QFile::exists(fn))
        {
            QMimeDatabase mimeDb;
            QMimeType mt = mimeDb.mimeTypeForFile(fn, QMimeDatabase::MatchContent);

            if (mt.isValid() &&
                    mt.inherits(QLatin1String("application/x-kwallet"))) {
                wm.openWalletFile(fn);
            }
            else {
                wm.openWallet(a.arguments().at(i));
            }
        }
    }

    return a.exec();
}

