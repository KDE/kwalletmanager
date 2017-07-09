project(kwalletmanager5)

# KDE Application Version, managed by release script
set (KDE_APPLICATIONS_VERSION_MAJOR "17")
set (KDE_APPLICATIONS_VERSION_MINOR "04")
set (KDE_APPLICATIONS_VERSION_MICRO "3")
set (KDE_APPLICATIONS_VERSION "${KDE_APPLICATIONS_VERSION_MAJOR}.${KDE_APPLICATIONS_VERSION_MINOR}.${KDE_APPLICATIONS_VERSION_MICRO}")

cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 1.7.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 5.2.0 CONFIG REQUIRED Core Gui Widgets DBus)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMSetupVersion)

## Generate header with version number
ecm_setup_version(${KDE_APPLICATIONS_VERSION} VARIABLE_PREFIX KWALLETMANAGER
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/kwalletmanager_version.h"
)

find_package(KF5 5.2.0 REQUIRED
    Archive
    Auth
    Config
    ConfigWidgets
    CoreAddons
    DBusAddons
    DocTools
    I18n
    IconThemes
    JobWidgets
    KCMUtils
    KIO
    Notifications
    Service
    TextWidgets
    Wallet
    WindowSystem
    XmlGui
)

add_subdirectory( doc )
add_subdirectory( icons )
add_subdirectory( src )

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

########### install files ###############
install( PROGRAMS org.kde.kwalletmanager5.desktop kwalletmanager5-kwalletd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
