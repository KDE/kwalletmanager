INCLUDES= $(all_includes)

SUBDIRS=. icons konfigurator

bin_PROGRAMS = kwalletmanager
kwalletmanager_SOURCES = walletwidget.ui kwalletmanager.cpp \
			kwalletmanager.skel main.cpp kwalletpopup.cpp \
			kwalleteditor.cpp kwmapeditor.cpp allyourbase.cpp
kwalletmanager_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kwalletmanager_LDADD   = $(LIB_KHTML) -lkwalletclient
METASOURCES = AUTO
noinst_HEADERS = kwalletmanager.h kwalletpopup.h walletwidget.h kwalleteditor.h\
		allyourbase.h kwmapeditor.h

xdg_apps_DATA = kwalletmanager.desktop

KDE_ICON = kwalletmanager

rc_DATA = kwalletmanager.rc kwalleteditor.rc
rcdir = $(kde_datadir)/kwalletmanager

messages: rc.cpp
        $(XGETTEXT) *.cpp -o $(podir)/kwallet.pot

