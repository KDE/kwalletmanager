########### next target ###############
add_executable(kwalletmanager5)
target_sources(kwalletmanager5 PRIVATE
    kwalletmanager.cpp
    kwalletmanagerwidget.cpp
    kwalletmanagerwidgetitem.cpp
    kwhexview.cpp
    walletcontrolwidget.cpp
    main.cpp
    kwalletpopup.cpp
    kwalleteditor.cpp
    kwmapeditor.cpp
    allyourbase.cpp
    kbetterthankdialogbase.cpp
    registercreateactionmethod.cpp
    applicationsmanager.cpp
    connectedapplicationstable.cpp
    connectedappmodel.cpp
    disconnectappbutton.cpp
    authorizedappmodel.cpp
    authorizedapplicationstable.cpp
    revokeauthbutton.cpp
    kwalletmanager.h
    kwalletmanagerwidget.h
    walletcontrolwidget.h
    allyourbase.h
    kwhexview.h
    authorizedapplicationstable.h
    kwalletmanagerwidgetitem.h
    connectedapplicationstable.h
    authorizedappmodel.h
    kwalleteditor.h
    connectedappmodel.h
    kwalletpopup.h
    kbetterthankdialogbase.h
    revokeauthbutton.h
    registercreateactionmethod.h
    disconnectappbutton.h
    kwmapeditor.h
    applicationsmanager.h

    kwalletmanager.qrc
)

ecm_qt_declare_logging_category(kwalletmanager5
                                HEADER kwalletmanager_debug.h
                                IDENTIFIER KWALLETMANAGER_LOG
				CATEGORY_NAME org.kde.kwalletmanager DESCRIPTION "kwalletmanager" EXPORT KWALLETMANAGER)

qt_add_dbus_interface(kwalletmanager_SRCS
    ${KWALLET_DBUS_INTERFACES_DIR}/kf${QT_MAJOR_VERSION}_org.kde.KWallet.xml kwallet_interface
)
target_sources(kwalletmanager5 PRIVATE ${kwalletmanager_SRCS})

ki18n_wrap_ui(kwalletmanager5
    walletwidget.ui
    kbetterthankdialogbase.ui
    walletcontrolwidget.ui
    applicationsmanager.ui
)


# Sets the icon on Windows and OSX
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-kwalletmanager.png")
ecm_add_app_icon(kwalletmanager5 ICONS ${ICONS_SRCS})


target_link_libraries(kwalletmanager5
    Qt${QT_MAJOR_VERSION}::Core
    KF${KF_MAJOR_VERSION}::CoreAddons
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::XmlGui #KActionCollection
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    KF${KF_MAJOR_VERSION}::ItemViews
    KF${KF_MAJOR_VERSION}::Wallet
    KF${KF_MAJOR_VERSION}::DBusAddons
    KF${KF_MAJOR_VERSION}::ConfigCore
    KF${KF_MAJOR_VERSION}::Service
    KF${KF_MAJOR_VERSION}::KIOCore
    KF${KF_MAJOR_VERSION}::KIOGui
    KF${KF_MAJOR_VERSION}::JobWidgets
    KF${KF_MAJOR_VERSION}::Archive
    KF${KF_MAJOR_VERSION}::Notifications
    KF${KF_MAJOR_VERSION}::WindowSystem
    KF${KF_MAJOR_VERSION}::Crash
    KF${KF_MAJOR_VERSION}::Codecs
)

install(TARGETS kwalletmanager5  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

add_subdirectory(icons)

########### install files ###############

if (QT_MAJOR_VERSION STREQUAL "5")
    install(FILES kwalletmanager5_show.desktop  DESTINATION ${KDE_INSTALL_KSERVICESDIR})
endif()
