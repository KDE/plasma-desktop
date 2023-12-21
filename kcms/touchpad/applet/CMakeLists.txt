remove_definitions(-DTRANSLATION_DOMAIN="kcm_touchpad")
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_touchpad")

plasma_install_package(qml touchpad)

set(Applet_SRCS
    touchpadengine.cpp
    touchpadservice.cpp
)

qt_generate_dbus_interface(../kded/kded.h org.kde.touchpad.xml)
qt_add_dbus_interfaces(Applet_SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)
qt_add_dbus_interfaces(Applet_SRCS
                        ${KDED_DBUS_INTERFACE}
)

kcoreaddons_add_plugin(plasma_engine_touchpad SOURCES ${Applet_SRCS} INSTALL_NAMESPACE "plasma5support/dataengine")

target_link_libraries(plasma_engine_touchpad
    Plasma::Plasma5Support
    Qt::DBus
)

install(FILES touchpad.operations
        DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services
)
install(FILES touchpad.svg
        DESTINATION ${KDE_INSTALL_DATADIR}/plasma/desktoptheme/default/icons
)
