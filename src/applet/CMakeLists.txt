add_subdirectory(qml)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dataengine.desktop"
               "${CMAKE_CURRENT_BINARY_DIR}/plasma-dataengine-touchpad.desktop"
)

set(Applet_SRCS
    touchpadengine.cpp
    touchpadservice.cpp
)

qt4_generate_dbus_interface(../kded/kded.h org.kde.touchpad.xml)
qt4_add_dbus_interfaces(Applet_SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)
qt4_add_dbus_interfaces(Applet_SRCS
                        ${KDE4_DBUS_INTERFACES_DIR}/org.kde.kded.xml
)

kde4_add_plugin(plasma_engine_touchpad ${Applet_SRCS})
target_link_libraries(plasma_engine_touchpad
                      ${KDE4_PLASMA_LIBS}
)

install(TARGETS plasma_engine_touchpad
        DESTINATION ${PLUGIN_INSTALL_DIR}
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-dataengine-touchpad.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
)
install(FILES touchpad.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services
)
install(FILES touchpad.svg
        DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/icons
)
