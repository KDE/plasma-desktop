

# Background test plasmoid
install(DIRECTORY org.kde.tests.background/ DESTINATION ${PLASMA_DATA_INSTALL_DIR}/plasmoids/org.kde.tests.background)
install(FILES org.kde.tests.background/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-org.kde.tests.background.desktop)

