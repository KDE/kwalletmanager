INCLUDES= $(all_includes)

bin_PROGRAMS = kwalletmanager
kwalletmanager_SOURCES = kwalletmanager.cpp kwalletmanager.skel main.cpp kwalletpopup.cpp
kwalletmanager_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kwalletmanager_LDADD   = $(LIB_KDEUI) -lkwalletclient
METASOURCES = AUTO
noinst_HEADERS = kwalletmanager.h kwalletpopup.h

xdg_apps_DATA = kwalletmanager.desktop

rc_DATA = kwalletmanager.rc
rcdir = $(kde_datadir)/kwalletmanager

