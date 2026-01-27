cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "12")
set (RELEASE_SERVICE_VERSION_MICRO "2")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")
project(kwalletmanager5 VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "6.7.0")
set(KF_MIN_VERSION "6.3.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

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
include(ECMDeprecationSettings)
include(KDEClangFormat)
include(KDEGitCommitHooks)

option(BUILD_DOC "Whether to build the documentation" ON)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED Core Gui Widgets DBus)

## Generate header with version number
ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX KWALLETMANAGER
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/kwalletmanager_version.h"
)

option(ENABLE_KAUTH "Enable KAuth support (Disabled for Flatpak builds)" ON)
if (ENABLE_KAUTH)
    find_package(KF6 ${KF_MIN_VERSION} REQUIRED Auth)
endif()

find_package(KF6 ${KF_MIN_VERSION} REQUIRED
    Archive
    Config
    ConfigWidgets
    CoreAddons
    DBusAddons
    I18n
    KCMUtils
    KIO
    Wallet
    WindowSystem
    XmlGui
    Crash
    StatusNotifierItem
    Service
)

if (BUILD_DOC)
    find_package(KF6DocTools ${KF_MIN_VERSION})
    set_package_properties(KF6DocTools PROPERTIES
        DESCRIPTION "Tools to generate documentation"
        TYPE REQUIRED
    )
endif()

ecm_set_disabled_deprecation_versions(QT 6.9.0
    KF 6.19.0
)

add_subdirectory( icons )
add_subdirectory( src )


########### install files ###############
install( PROGRAMS org.kde.kwalletmanager.desktop kwalletmanager5-kwalletd.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kwalletmanager5.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
ecm_generate_dbus_service_file(
    NAME org.kde.kwalletmanager
    EXECUTABLE ${KDE_INSTALL_FULL_BINDIR}/kwalletmanager5
    DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR}
)

ecm_qt_install_logging_categories(
	EXPORT KWALLETMANAGER
        FILE kwalletmanager.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

if (BUILD_DOC)
    add_subdirectory(doc)
    kdoctools_install(po)
endif()

ki18n_install(po)
feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
