INCLUDES = $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kcm_kwallet.la
kcm_kwallet_la_SOURCES = walletconfigwidget.ui konfigurator.cpp
kcm_kwallet_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_kwallet_la_LIBADD = $(LIB_KDEUI)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmkwallet.pot

data_DATA = kwalletconfig.desktop
datadir = $(kde_appsdir)/Settings/Security

