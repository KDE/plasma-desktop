########### install files ###############
file(GLOB desktop_file "*.desktop")

install( FILES ${desktop_file}
    DESTINATION ${DATA_INSTALL_DIR}/ksmserver/windowmanagers )
