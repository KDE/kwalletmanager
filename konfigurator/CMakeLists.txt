



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

