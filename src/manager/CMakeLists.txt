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
    ${KWALLET_DBUS_INTERFACES_DIR}/kf6_org.kde.KWallet.xml kwallet_interface
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
    Qt6::Core
    KF6::CoreAddons
    KF6::I18n
    KF6::XmlGui #KActionCollection
    KF6::ConfigWidgets
    KF6::ItemViews
    KF6::Wallet
    KF6::DBusAddons
    KF6::ConfigCore
    KF6::Service
    KF6::KIOCore
    KF6::KIOGui
    KF6::JobWidgets
    KF6::Archive
    KF6::Notifications
    KF6::WindowSystem
    KF6::Crash
    KF6::Codecs
    KF6::StatusNotifierItem
)

install(TARGETS kwalletmanager5  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

add_subdirectory(icons)

########### install files ###############

