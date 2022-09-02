cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "22")
set (RELEASE_SERVICE_VERSION_MINOR "08")
set (RELEASE_SERVICE_VERSION_MICRO "1")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(kwalletmanager5 VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.90.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(FeatureSummary)
include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(ECMGenerateDBusServiceFile)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED Core Gui Widgets DBus)

## Generate header with version number
ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX KWALLETMANAGER
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/kwalletmanager_version.h"
)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED
    Archive
    Auth
    Config
    ConfigWidgets
    CoreAddons
    DBusAddons
    DocTools
    I18n
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

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050f00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055900
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

add_subdirectory( doc )
add_subdirectory( icons )
add_subdirectory( src )


########### install files ###############
install( PROGRAMS org.kde.kwalletmanager5.desktop kwalletmanager5-kwalletd.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kwalletmanager5.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
ecm_generate_dbus_service_file(
    NAME org.kde.kwalletmanager5
    EXECUTABLE ${KDE_INSTALL_FULL_BINDIR}/kwalletmanager5
    DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR}
)

ecm_qt_install_logging_categories(
	EXPORT KWALLETMANAGER
        FILE kwalletmanager.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )
ki18n_install(po)
kdoctools_install(po)
feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
