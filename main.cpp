/*
 * Copyright (C) 2003 George Staikos <staikos@kde.org>
 */

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "kwalletmanager.h"

int main(int argc, char **argv) {
static KCmdLineOptions options[] = { { 0, 0, 0 } };

KAboutData about("kwalletmanager", I18N_NOOP("kwalletmanager"), "1.0",
		I18N_NOOP("KDE Wallet Management Tool"),
		KAboutData::License_GPL,
		I18N_NOOP("(c) 2003 George Staikos"), 0,
		"http://www.kde.org/");

	about.addAuthor("George Staikos", "Primary author and maintainer", "staikos@kde.org");
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication a;
	//KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	KWalletManager wm;
	wm.setCaption(i18n("KDE Wallet Manager"));

	a.setMainWidget(&wm);

return a.exec();
}

