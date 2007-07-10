



########### next target ###############

set(kcm_kwallet_PART_SRCS konfigurator.cpp )

kde4_add_ui_files(kcm_kwallet_PART_SRCS walletconfigwidget.ui )

kde4_add_plugin(kcm_kwallet ${kcm_kwallet_PART_SRCS})


target_link_libraries(kcm_kwallet  ${KDE4_KDE3SUPPORT_LIBS}  )

install(TARGETS kcm_kwallet  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kwalletconfig.desktop kwalletmanager_show.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
