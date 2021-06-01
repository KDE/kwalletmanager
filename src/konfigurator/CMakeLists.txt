########### next target ###############
add_definitions(-DTRANSLATION_DOMAIN=\"kcmkwallet\")

add_library(kcm_kwallet5 MODULE)
target_sources(kcm_kwallet5 PRIVATE konfigurator.cpp )

ki18n_wrap_ui(kcm_kwallet5 walletconfigwidget.ui )



target_link_libraries(kcm_kwallet5
    Qt::Core
    KF5::AuthCore
    KF5::CoreAddons
    KF5::KCMUtils
    KF5::Wallet
    KF5::I18n
    Qt::DBus
)

install(TARGETS kcm_kwallet5  DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### kauth helper ################
add_executable(kcm_kwallet_helper5)
target_sources(kcm_kwallet_helper5 PRIVATE savehelper.cpp)

target_link_libraries(kcm_kwallet_helper5
    Qt::Core
    KF5::AuthCore
    KF5::Wallet
    KF5::CoreAddons
)

install(TARGETS kcm_kwallet_helper5 DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(kcm_kwallet_helper5 org.kde.kcontrol.kcmkwallet5 root)
kauth_install_actions(org.kde.kcontrol.kcmkwallet5 kwallet.actions)

########### install files ###############

install( FILES kwalletconfig5.desktop kwalletmanager5_show.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
