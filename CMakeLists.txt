project(kwalletmanager5)

cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 1.2.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 5.2.0 CONFIG REQUIRED Core Gui Widgets DBus)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)
include(ECMInstallIcons)

find_package(KF5 5.2.0 REQUIRED
    CoreAddons
    DocTools
    Auth
    Wallet
    Service
    KCMUtils
    KDELibs4Support
    I18n
    XmlGui
    Config
    ConfigWidgets
    DBusAddons
)

add_subdirectory( doc )
add_subdirectory( icons )
add_subdirectory( src )

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

########### install files ###############
install( PROGRAMS org.kde.kwalletmanager5.desktop kwalletmanager5-kwalletd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

