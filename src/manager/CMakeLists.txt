########### next target ###############
set(kwalletmanager_SRCS 
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
)

qt5_add_dbus_interface(kwalletmanager_SRCS
    ${KWALLET_DBUS_INTERFACES_DIR}/kf5_org.kde.KWallet.xml kwallet_interface
)

ki18n_wrap_ui(kwalletmanager_SRCS
    walletwidget.ui
    kbetterthankdialogbase.ui
    walletcontrolwidget.ui
    applicationsmanager.ui
)


# Sets the icon on Windows and OSX
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*-apps-kwalletmanager.png")
ecm_add_app_icon(kwalletmanager_SRCS ICONS ${ICONS_SRCS})

add_executable(kwalletmanager5 ${kwalletmanager_SRCS})

target_link_libraries(kwalletmanager5
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui #KActionCollection
    KF5::ConfigWidgets
    KF5::ItemViews
    KF5::Wallet
    KF5::DBusAddons
    KF5::ConfigCore
    KF5::Service
    KF5::KIOCore
    KF5::JobWidgets
    KF5::Archive
    KF5::IconThemes
    KF5::Notifications
    KF5::WindowSystem
    KF5::Crash
)

install(TARGETS kwalletmanager5  ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES kwalletmanager.rc
        DESTINATION ${KXMLGUI_INSTALL_DIR}/kwalletmanager5)

ecm_install_icons( ICONS
128-apps-kwalletmanager2.png
128-apps-kwalletmanager.png
16-apps-kwalletmanager2.png
16-apps-kwalletmanager.png
22-apps-kwalletmanager.png
32-apps-kwalletmanager2.png
32-apps-kwalletmanager.png
48-apps-kwalletmanager2.png
48-apps-kwalletmanager.png
64-apps-kwalletmanager2.png
64-apps-kwalletmanager.png
DESTINATION ${ICON_INSTALL_DIR} )
