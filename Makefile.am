INCLUDES= $(all_includes)

SUBDIRS=. icons konfigurator

bin_PROGRAMS = kwalletmanager
kwalletmanager_SOURCES = walletwidget.ui kwalletmanager.cpp \
			kwalletmanager.skel main.cpp kwalletpopup.cpp \
			kwalleteditor.cpp kwmapeditor.cpp allyourbase.cpp \
			kbetterthankdialogbase.ui
kwalletmanager_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kwalletmanager_LDADD   = $(LIB_KIO) -lkwalletclient
METASOURCES = AUTO
noinst_HEADERS = kwalletmanager.h kwalletpopup.h walletwidget.h kwalleteditor.h\
		allyourbase.h kwmapeditor.h

xdg_apps_DATA = kwalletmanager.desktop kwalletmanager-kwalletd.desktop

KDE_ICON = kwalletmanager

rc_DATA = kwalletmanager.rc kwalleteditor.rc
rcdir = $(kde_datadir)/kwalletmanager

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kwalletmanager.pot

kbetterthankdialogbase.lo: kbetterthankdialogbase.ui kbetterthankdialogbase.ui.h
