file(GLOB desktopfile *.desktop)

install( FILES ${desktopfile} DESTINATION ${DATA_INSTALL_DIR}/solid/devices )
