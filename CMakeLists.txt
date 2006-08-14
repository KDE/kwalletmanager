
add_subdirectory( icons ) 
add_subdirectory( konfigurator ) 




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


kde4_add_executable(kwalletmanager ${kwalletmanager_SRCS})

target_link_libraries(kwalletmanager  ${KDE4_KIO_LIBS} kwalletclient )

install(TARGETS kwalletmanager  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kwalletmanager.desktop kwalletmanager-kwalletd.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES kwalletmanager.rc kwalleteditor.rc  DESTINATION ${DATA_INSTALL_DIR}/kwalletmanager)

kde4_install_icons( ${ICON_INSTALL_DIR}   )




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
