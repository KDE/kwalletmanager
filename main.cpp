/*
   Copyright (C) 2003 George Staikos <staikos@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kuniqueapplication.h>

#include "kwalletmanager.h"


class MyApp : public KUniqueApplication {
	public:
		MyApp() : KUniqueApplication() {}
		virtual ~MyApp() {}

		virtual int newInstance() { return 0; }
};

int main(int argc, char **argv) {
static KCmdLineOptions options[] = {
	{"show", I18N_NOOP("Show window on startup."), 0},
	{"+name", I18N_NOOP("A wallet name."), 0},
	KCmdLineLastOption
};

KAboutData about("kwalletmanager", I18N_NOOP("kwalletmanager"), "1.0",
		I18N_NOOP("KDE Wallet Management Tool"),
		KAboutData::License_GPL,
		I18N_NOOP("(c) 2003 George Staikos"), 0,
		"http://www.kde.org/");

	about.addAuthor("George Staikos", "Primary author and maintainer", "staikos@kde.org");

	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(options);

	if (!KUniqueApplication::start()) {
		return 0;
	}

	MyApp a;

	KWalletManager wm;
	wm.setCaption(i18n("KDE Wallet Manager"));

	a.setMainWidget(&wm);

	KGlobal::dirs()->addResourceType("kwallet", "share/apps/kwallet");

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	if (args->isSet("show")) {
		wm.show();
	}

	for (int i = 0; i < args->count(); ++i) {
		wm.openWallet(args->arg(i));
	}

return a.exec();
}

