INCLUDES = $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kcm_kwallet.la
kcm_kwallet_la_SOURCES = walletconfigwidget.ui konfigurator.cpp
kcm_kwallet_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_kwallet_la_LIBADD = $(LIB_KDEUI) -lkwalletclient

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmkwallet.pot

xdg_apps_DATA = kwalletconfig.desktop

kde_services_DATA = kwallet_config.desktop kwalletmanager_show.desktop

