project(kwallet)
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

kde4_automoc(${kwalletmanager_SRCS})

kde4_add_ui_files(kwalletmanager_SRCS walletwidget.ui kbetterthankdialogbase.ui)

kde4_add_executable(kwalletmanager ${kwalletmanager_SRCS})

target_link_libraries(kwalletmanager   ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kwalletmanager  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kwalletmanager.desktop kwalletmanager-kwalletd.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES kwalletmanager.rc kwalleteditor.rc  DESTINATION ${DATA_INSTALL_DIR}/kwalletmanager)

kde4_install_icons( ${ICON_INSTALL_DIR}   )

