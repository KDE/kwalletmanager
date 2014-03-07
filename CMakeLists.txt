project(kwalletmanager)

cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(REQUIRED_QT_VERSION 5.2.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Gui Widgets DBus Test)

option(BUILD_TESTS "Build KWalletManager with unit tests" ON)

set(KF5_VERSION "4.96.0")
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

if(NOT kdelibs_SOURCE_DIR)
    find_package(KF5 ${KF5_VERSION} REQUIRED
        CoreAddons
        DocTools
#        WidgetsAddons
#        Service
#        Config
#        WindowSystem
#        I18n
#        KDE4Support #KUniqueApplication in ksecretsserviced
    )
endif()

add_subdirectory( doc )
add_subdirectory( icons ) 
add_subdirectory( src ) 

if("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()

########### install files ###############
install( PROGRAMS kwalletmanager.desktop kwalletmanager-kwalletd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})

