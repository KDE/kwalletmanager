INCLUDES= $(all_includes)

bin_PROGRAMS = kwalletmanager
kwalletmanager_SOURCES = kwalletmanager.cpp kwalletmanager.skel main.cpp \
			kwalletpopup.cpp kwalleteditor.cpp walletwidget.ui \
			allyourbase.cpp
kwalletmanager_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kwalletmanager_LDADD   = $(LIB_KHTML) -lkwalletclient
METASOURCES = AUTO
noinst_HEADERS = kwalletmanager.h kwalletpopup.h walletwidget.h kwalleteditor.h\
		allyourbase.h

xdg_apps_DATA = kwalletmanager.desktop

rc_DATA = kwalletmanager.rc kwalleteditor.rc
rcdir = $(kde_datadir)/kwalletmanager

