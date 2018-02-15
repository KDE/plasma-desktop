file(GLOB desktopfile *.desktop)

install( FILES ${desktopfile} DESTINATION ${KDE_INSTALL_DATADIR}/solid/devices )
