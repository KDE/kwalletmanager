kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_kwallet_PART_SRCS konfigurator.cpp )

kde4_automoc(${kcm_kwallet_PART_SRCS})

kde4_add_ui3_files(kcm_kwallet_PART_SRCS walletconfigwidget.ui )

kde4_add_plugin(kcm_kwallet ${kcm_kwallet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kwallet )

target_link_libraries(kcm_kwallet  ${LIB_KDE3SUPPORT} kwalletclient )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kwallet )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kwalletconfig.desktop )
install_files( ${SERVICES_INSTALL_DIR} FILES kwallet_config.desktop kwalletmanager_show.desktop )

kde4_footer()



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
