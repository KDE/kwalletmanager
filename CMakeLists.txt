project(kwallet)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4)
    include( KDE4Defaults )
    include_directories(${KDE4_INCLUDES})

    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
    add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

    # If definitions like -D_GNU_SOURCE are needed for these checks they
    # should be added to _KDE4_PLATFORM_DEFINITIONS when it is originally
    # defined outside this file.  Here we include these definitions in
    # CMAKE_REQUIRED_DEFINITIONS so they will be included in the build of
    # checks below.
    set( CMAKE_REQUIRED_DEFINITIONS ${_KDE4_PLATFORM_DEFINITIONS} )
endif()

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory( doc )
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

target_link_libraries(kwalletmanager ${KDE4_KIO_LIBS})

install(TARGETS kwalletmanager  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS kwalletmanager.desktop kwalletmanager-kwalletd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kwalletmanager.rc kwalleteditor.rc  DESTINATION ${DATA_INSTALL_DIR}/kwalletmanager)

kde4_install_icons( ${ICON_INSTALL_DIR}   )

