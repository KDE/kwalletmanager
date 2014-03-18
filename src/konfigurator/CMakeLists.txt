########### next target ###############

set(kcm_kwallet_PART_SRCS konfigurator.cpp )

qt5_wrap_ui(kcm_kwallet_PART_SRCS walletconfigwidget.ui )

add_library(kcm_kwallet MODULE ${kcm_kwallet_PART_SRCS})


target_link_libraries(kcm_kwallet
    Qt5::Core
    KF5::Auth
    KF5::CoreAddons
    KF5::KCMUtils
    KF5::Wallet
    KF5::KDE4Support
)

install(TARGETS kcm_kwallet  DESTINATION ${PLUGIN_INSTALL_DIR})

########### kauth helper ################
set(kcm_kwallet_helper_SRCS savehelper.cpp)

add_executable(kcm_kwallet_helper ${kcm_kwallet_helper_SRCS})
target_link_libraries(kcm_kwallet_helper
    Qt5::Core
    KF5::Auth
    KF5::Wallet
    KF5::CoreAddons
)

install(TARGETS kcm_kwallet_helper DESTINATION ${LIBEXEC_INSTALL_DIR})

kauth_install_helper_files(kcm_kwallet_helper org.kde.kcontrol.kcmkwallet root)
kauth_install_actions(org.kde.kcontrol.kcmkwallet kwallet.actions)

########### install files ###############

install( FILES kwalletconfig.desktop kwalletmanager_show.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
