########### next target ###############

set(kwalletmanager_SRCS 
    kwalletmanager.cpp
    kwalletmanagerwidget.cpp
    kwalletmanagerwidgetitem.cpp
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

qt5_wrap_ui(kwalletmanager_SRCS
    walletwidget.ui
    kbetterthankdialogbase.ui
    walletcontrolwidget.ui
    applicationsmanager.ui
)

# Sets the icon on Windows and OSX, TODO: port to KF5
#kde4_add_app_icon(kwalletmanager_SRCS
#    "${CMAKE_CURRENT_SOURCE_DIR}/hi*-app-kwalletmanager.png"
#)

add_executable(kwalletmanager ${kwalletmanager_SRCS})

target_link_libraries(kwalletmanager
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui #KActionCollection
    KF5::ConfigWidgets
    KF5::ItemViews
    KF5::Wallet
    KF5::DBusAddons
    KF5::ConfigCore
    KF5::KDE4Support
)

install(TARGETS kwalletmanager  ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES kwalletmanager.rc
        DESTINATION ${DATA_INSTALL_DIR}/kwalletmanager)

ecm_install_icons( ${ICON_INSTALL_DIR} )
