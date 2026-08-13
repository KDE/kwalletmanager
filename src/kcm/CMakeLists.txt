kcmutils_add_qml_kcm(kcm_kwallet)

target_sources(kcm_kwallet PRIVATE kcm.cpp kcm.h)

kcmutils_generate_module_data(kcm_kwallet
    MODULE_DATA_HEADER kwalletdata.h
    MODULE_DATA_CLASS_NAME KWalletData
    SETTINGS_HEADERS kwalletsettings.h
    SETTINGS_CLASSES KWalletSettings
)
kconfig_target_kcfg_file(kcm_kwallet FILE ${KWALLET_KCFG_FILE} CLASS_NAME KWalletSettings PARENT_IN_CONSTRUCTOR GENERATE_PROPERTIES GENERATE_MOC MUTATORS DEFAULT_VALUE_GETTERS)

target_link_libraries(kcm_kwallet PRIVATE Qt::DBus KF6::CoreAddons KF6::I18n KF6::ConfigGui KF6::KCMUtilsQuick KF6::Wallet KF6::AuthCore)

########### kauth helper ################
add_executable(kcm_kwallet_helper5)
target_sources(kcm_kwallet_helper5 PRIVATE savehelper.cpp savehelper.h)

target_link_libraries(kcm_kwallet_helper5
    Qt6::Core
    KF6::AuthCore
    KF6::Wallet
    KF6::CoreAddons
)

install(TARGETS kcm_kwallet_helper5 DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(kcm_kwallet_helper5 org.kde.kcontrol.kcmkwallet5 root)
kauth_install_actions(org.kde.kcontrol.kcmkwallet5 kwallet.actions)
