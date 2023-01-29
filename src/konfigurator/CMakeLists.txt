########### next target ###############
add_definitions(-DTRANSLATION_DOMAIN=\"kcmkwallet\")

add_library(kcm_kwallet5 MODULE)
target_sources(kcm_kwallet5 PRIVATE konfigurator.cpp konfigurator.h)

ki18n_wrap_ui(kcm_kwallet5 walletconfigwidget.ui )



target_link_libraries(kcm_kwallet5
    Qt${QT_MAJOR_VERSION}::Core
    KF${KF_MAJOR_VERSION}::AuthCore
    KF${KF_MAJOR_VERSION}::CoreAddons
    KF${KF_MAJOR_VERSION}::KCMUtils
    KF${KF_MAJOR_VERSION}::Wallet
    KF${KF_MAJOR_VERSION}::I18n
    Qt${QT_MAJOR_VERSION}::DBus
)

install(TARGETS kcm_kwallet5  DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings_qwidgets)

########### kauth helper ################
add_executable(kcm_kwallet_helper5)
target_sources(kcm_kwallet_helper5 PRIVATE savehelper.cpp savehelper.h)

target_link_libraries(kcm_kwallet_helper5
    Qt${QT_MAJOR_VERSION}::Core
    KF${KF_MAJOR_VERSION}::AuthCore
    KF${KF_MAJOR_VERSION}::Wallet
    KF${KF_MAJOR_VERSION}::CoreAddons
)

install(TARGETS kcm_kwallet_helper5 DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(kcm_kwallet_helper5 org.kde.kcontrol.kcmkwallet5 root)
kauth_install_actions(org.kde.kcontrol.kcmkwallet5 kwallet.actions)

