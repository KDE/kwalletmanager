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

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kuniqueapplication.h>

#include <QFile>
#include <QFileInfo>

#include "kwalletmanager.h"


class MyApp : public KUniqueApplication {
	public:
		MyApp() : KUniqueApplication() { KGlobal::ref(); }
		virtual ~MyApp() {}

		virtual int newInstance() { return 0; }
};

int main(int argc, char **argv) {
	KAboutData about("kwalletmanager", 0, ki18n("KDE Wallet Manager"), "2.0",
		ki18n("KDE Wallet Management Tool"),
		KAboutData::License_GPL,
		ki18n("(c) 2003,2004 George Staikos"), KLocalizedString(),
		"http://utils.kde.org/projects/kwalletmanager");

    about.addAuthor(ki18n("Valentin Rusu"), ki18n("Maintainer, user interface refactoring"), "kde@rusu.info");
	about.addAuthor(ki18n("George Staikos"), ki18n("Original author and former maintainer"), "staikos@kde.org");
    about.addAuthor(ki18n("Michael Leupold"), ki18n("Developer and former maintainer"), "lemma@confuego.org");
	about.addAuthor(ki18n("Isaac Clerencia"), ki18n("Developer"), "isaac@warp.es");

	KCmdLineArgs::init(argc, argv, &about);

	KCmdLineOptions options;
	options.add("show", ki18n("Show window on startup"));
	options.add("kwalletd", ki18n("For use by kwalletd only"));
	options.add("+name", ki18n("A wallet name"));
	KCmdLineArgs::addCmdLineOptions(options);

	if (!KUniqueApplication::start()) {
		return 0;
	}

	MyApp a;

	KWalletManager wm;
	wm.setCaption(i18n("KDE Wallet Manager"));

	KGlobal::dirs()->addResourceType("kwallet", 0, QLatin1String( "share/apps/kwallet" ));

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	if (args->isSet("show")) {
		wm.show();
	}

	if (args->isSet("kwalletd")) {
		wm.kwalletdLaunch();
	}

	for (int i = 0; i < args->count(); ++i) {
		QString fn = QFileInfo(args->arg(i)).absoluteFilePath();
		KMimeType::Ptr ptr;
		if (QFile::exists(fn) &&
			(ptr = KMimeType::findByFileContent(fn)) &&
			ptr->is(QLatin1String( "application/x-kwallet" ))) {
			wm.openWalletFile(fn);
		} else {
			wm.openWallet(args->arg(i));
		}
	}
	args->clear();
	return a.exec();
}

