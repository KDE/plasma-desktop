remove_definitions(-DTRANSLATION_DOMAIN="kcm_touchpad")
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_touchpad")

plasma_install_package(qml touchpad)

set(Applet_SRCS
    touchpadengine.cpp
    touchpadservice.cpp
)

qt5_generate_dbus_interface(../kded/kded.h org.kde.touchpad.xml)
qt5_add_dbus_interfaces(Applet_SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)
qt5_add_dbus_interfaces(Applet_SRCS
                        ${KDED_DBUS_INTERFACE}
)

add_library(plasma_engine_touchpad MODULE ${Applet_SRCS})
target_link_libraries(plasma_engine_touchpad
                      KF5::Plasma
		      Qt::DBus
)

kcoreaddons_desktop_to_json(plasma_engine_touchpad plasma-dataengine-touchpad.desktop SERVICE_TYPES kcmodule.desktop)

install(TARGETS plasma_engine_touchpad
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-dataengine-touchpad.desktop
        DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
install(FILES touchpad.operations
        DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services
)
install(FILES touchpad.svg
        DESTINATION ${KDE_INSTALL_DATADIR}/plasma/desktoptheme/default/icons
)
