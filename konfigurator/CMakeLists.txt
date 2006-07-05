



########### next target ###############

set(kcm_kwallet_PART_SRCS konfigurator.cpp )

kde4_automoc(${kcm_kwallet_PART_SRCS})

kde4_add_ui3_files(kcm_kwallet_PART_SRCS walletconfigwidget.ui )

kde4_add_plugin(kcm_kwallet ${kcm_kwallet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kwallet )

target_link_libraries(kcm_kwallet  ${KDE4_KDE3SUPPORT_LIBS} kwalletclient )

install(TARGETS kcm_kwallet  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kwalletconfig.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES kwallet_config.desktop kwalletmanager_show.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = kcm_kwallet.la
#kcm_kwallet_la_SOURCES = walletconfigwidget.ui konfigurator.cpp
#kcm_kwallet_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
#kcm_kwallet_la_LIBADD = $(LIB_KDEUI) -lkwalletclient
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmkwallet.pot
#
#xdg_apps_DATA = kwalletconfig.desktop
#
#kde_services_DATA = kwallet_config.desktop kwalletmanager_show.desktop
#
