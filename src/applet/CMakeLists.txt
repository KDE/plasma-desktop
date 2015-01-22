remove_definitions(-DTRANSLATION_DOMAIN="kcm_touchpad")
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_touchpad")

add_subdirectory(qml)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dataengine.desktop"
               "${CMAKE_CURRENT_BINARY_DIR}/plasma-dataengine-touchpad.desktop"
)

set(Applet_SRCS
    touchpadengine.cpp
    touchpadservice.cpp
)

qt5_generate_dbus_interface(../kded/kded.h org.kde.touchpad.xml)
qt5_add_dbus_interfaces(Applet_SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)
qt5_add_dbus_interfaces(Applet_SRCS
                        ${KINIT_DBUS_INTERFACES_DIR}/org.kde.kded5.xml
)

add_library(plasma_engine_touchpad MODULE ${Applet_SRCS})
target_link_libraries(plasma_engine_touchpad
                      KF5::Plasma
		      Qt5::DBus
)

kcoreaddons_desktop_to_json(plasma_engine_touchpad ${CMAKE_CURRENT_BINARY_DIR}/plasma-dataengine-touchpad.desktop)

install(TARGETS plasma_engine_touchpad
        DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-dataengine-touchpad.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
)
install(FILES touchpad.operations
        DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services
)
install(FILES touchpad.svg
        DESTINATION ${DATA_INSTALL_DIR}/plasma/desktoptheme/default/icons
)
