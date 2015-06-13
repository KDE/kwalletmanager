########### next target ###############
add_definitions(-DTRANSLATION_DOMAIN=\"kcmkwallet\")

set(kcm_kwallet_PART_SRCS konfigurator.cpp )

ki18n_wrap_ui(kcm_kwallet_PART_SRCS walletconfigwidget.ui )

add_library(kcm_kwallet5 MODULE ${kcm_kwallet_PART_SRCS})


target_link_libraries(kcm_kwallet5
    Qt5::Core
    KF5::Auth
    KF5::CoreAddons
    KF5::KCMUtils
    KF5::Wallet
    KF5::I18n
    Qt5::DBus
)

install(TARGETS kcm_kwallet5  DESTINATION ${PLUGIN_INSTALL_DIR})

########### kauth helper ################
set(kcm_kwallet_helper_SRCS savehelper.cpp)

add_executable(kcm_kwallet_helper5 ${kcm_kwallet_helper_SRCS})
target_link_libraries(kcm_kwallet_helper5
    Qt5::Core
    KF5::Auth
    KF5::Wallet
    KF5::CoreAddons
)

install(TARGETS kcm_kwallet_helper5 DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(kcm_kwallet_helper5 org.kde.kcontrol.kcmkwallet5 root)
kauth_install_actions(org.kde.kcontrol.kcmkwallet5 kwallet.actions)

########### install files ###############

install( FILES kwalletconfig5.desktop kwalletmanager5_show.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
