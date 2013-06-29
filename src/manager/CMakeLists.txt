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

qt4_add_dbus_interface(kwalletmanager_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KWallet.xml kwallet_interface)

kde4_add_ui_files(kwalletmanager_SRCS walletwidget.ui kbetterthankdialogbase.ui walletcontrolwidget.ui applicationsmanager.ui)

kde4_add_app_icon(kwalletmanager_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-app-kwalletmanager.png")

kde4_add_executable(kwalletmanager ${kwalletmanager_SRCS})

target_link_libraries(kwalletmanager ${KDE4_KIO_LIBS})

install(TARGETS kwalletmanager  ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES kwalletmanager.rc DESTINATION ${DATA_INSTALL_DIR}/kwalletmanager)

kde4_install_icons( ${ICON_INSTALL_DIR} )
