kde4_header()

add_subdirectory( icons ) 
add_subdirectory( konfigurator ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kwalletmanager_SRCS 
   kwalletmanager.cpp 
   main.cpp 
   kwalletpopup.cpp 
   kwalleteditor.cpp 
   kwmapeditor.cpp 
   allyourbase.cpp )

kde4_automoc(${kwalletmanager_SRCS})

kde4_add_ui3_files(kwalletmanager_SRCS walletwidget.ui kbetterthankdialogbase.ui )

kde4_add_dcop_skels(kwalletmanager_SRCS kwalletmanager.h )

kde4_add_executable(kwalletmanager ${kwalletmanager_SRCS})

target_link_libraries(kwalletmanager  ${QT_AND_KDECORE_LIBS} kio kwalletclient )

install_targets(/bin kwalletmanager )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kwalletmanager.desktop kwalletmanager-kwalletd.desktop )
install_files( ${DATA_INSTALL_DIR}/kwalletmanager FILES kwalletmanager.rc kwalleteditor.rc )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES= $(all_includes)
#
#SUBDIRS=. icons konfigurator
#
#bin_PROGRAMS = kwalletmanager
#kwalletmanager_SOURCES = walletwidget.ui kwalletmanager.cpp \
#			kwalletmanager.skel main.cpp kwalletpopup.cpp \
#			kwalleteditor.cpp kwmapeditor.cpp allyourbase.cpp \
#			kbetterthankdialogbase.ui
#kwalletmanager_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kwalletmanager_LDADD   = $(LIB_KDECORE) $(LIB_KIO) -lkwalletclient
#METASOURCES = AUTO
#noinst_HEADERS = kwalletmanager.h kwalletpopup.h walletwidget.h kwalleteditor.h\
#		allyourbase.h kwmapeditor.h
#
#xdg_apps_DATA = kwalletmanager.desktop kwalletmanager-kwalletd.desktop
#
#KDE_ICON = kwalletmanager
#
#rc_DATA = kwalletmanager.rc kwalleteditor.rc
#rcdir = $(kde_datadir)/kwalletmanager
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kwalletmanager.pot
#
#kbetterthankdialogbase.lo: kbetterthankdialogbase.ui kbetterthankdialogbase.ui.h
