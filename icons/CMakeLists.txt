kde4_header()




########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kwalletmanager/icons FILES  )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

#KDE_ICON = kwalletmanager
#
#pics_ICON = AUTO
#picsdir = $(kde_datadir)/kwalletmanager/icons
#
