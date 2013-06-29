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

add_subdirectory( doc )
add_subdirectory( icons ) 
add_subdirectory( src ) 


########### install files ###############

install( PROGRAMS kwalletmanager.desktop kwalletmanager-kwalletd.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
