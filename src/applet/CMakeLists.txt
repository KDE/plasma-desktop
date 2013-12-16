configure_file("${CMAKE_CURRENT_SOURCE_DIR}/touchpad-applet.desktop.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/touchpad-applet.desktop"
)

set(Applet_SRCS
    trayicon.cpp
)

qt4_add_dbus_interfaces(Applet_SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/../org.kde.touchpad.xml
)

kde4_add_plugin(touchpad_applet ${Applet_SRCS})
target_link_libraries(touchpad_applet
                      ${KDE4_PLASMA_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KUTILS_LIBS}
)

install(TARGETS touchpad_applet
        DESTINATION ${PLUGIN_INSTALL_DIR}
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/touchpad-applet.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
)
install(FILES touchpad.svg
        DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/icons
)
