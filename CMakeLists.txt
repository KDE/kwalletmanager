project(kwallet)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory( icons ) 
add_subdirectory( konfigurator ) 

########### next target ###############

set(kwalletmanager_SRCS 
   kwalletmanager.cpp 
   main.cpp 
   kwalletpopup.cpp 
   kwalleteditor.cpp 
   kwmapeditor.cpp 
   allyourbase.cpp 
   kbetterthankdialogbase.cpp )

qt4_add_dbus_interface(kwalletmanager_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KWallet.xml kwallet_interface)

kde4_add_ui_files(kwalletmanager_SRCS walletwidget.ui kbetterthankdialogbase.ui)

kde4_add_app_icon(kwalletmanager_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-app-kwalletmanager.png")

kde4_add_executable(kwalletmanager ${kwalletmanager_SRCS})

target_link_libraries(kwalletmanager ${LIBKNOTIFICATIONITEM-1_LIBRARIES} ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kwalletmanager  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES kwalletmanager.desktop kwalletmanager-kwalletd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kwalletmanager.rc kwalleteditor.rc  DESTINATION ${DATA_INSTALL_DIR}/kwalletmanager)

kde4_install_icons( ${ICON_INSTALL_DIR}   )

