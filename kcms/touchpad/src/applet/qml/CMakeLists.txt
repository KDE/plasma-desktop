configure_file(${CMAKE_CURRENT_SOURCE_DIR}/metadata.desktop
               ${CMAKE_CURRENT_BINARY_DIR}/metadata.desktop
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/metadata.desktop
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/touchpad
)

install(DIRECTORY contents/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/touchpad/contents
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-touchpad.desktop
)
