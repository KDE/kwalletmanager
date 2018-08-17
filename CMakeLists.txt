cmake_minimum_required(VERSION 3.0)

# KDE Application Version, managed by release script
set (KDE_APPLICATIONS_VERSION_MAJOR "18")
set (KDE_APPLICATIONS_VERSION_MINOR "11")
set (KDE_APPLICATIONS_VERSION_MICRO "70")
set (KDE_APPLICATIONS_VERSION "${KDE_APPLICATIONS_VERSION_MAJOR}.${KDE_APPLICATIONS_VERSION_MINOR}.${KDE_APPLICATIONS_VERSION_MICRO}")
project(kwalletmanager5 VERSION ${KDE_APPLICATIONS_VERSION})

set(QT_MIN_VERSION "5.9.0")
set(KF5_VERSION "5.46.0")

find_package(ECM ${KF5_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Gui Widgets DBus)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)

## Generate header with version number
ecm_setup_version(${KDE_APPLICATIONS_VERSION} VARIABLE_PREFIX KWALLETMANAGER
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/kwalletmanager_version.h"
)

find_package(KF5 ${KF5_VERSION} REQUIRED
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
    Crash
)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)

add_subdirectory( doc )
add_subdirectory( icons )
add_subdirectory( src )


########### install files ###############
install( PROGRAMS org.kde.kwalletmanager5.desktop kwalletmanager5-kwalletd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES org.kde.kwalletmanager5.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

install( FILES kwalletmanager.categories DESTINATION ${KDE_INSTALL_CONFDIR} )


feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
