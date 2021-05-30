set(DataEngine_SRCS
    touchpadengine.cpp
    touchpadservice.cpp
)

qt5_generate_dbus_interface(../kded/kded.h org.kde.touchpad.xml)
qt_add_dbus_interfaces(DataEngine_SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)
qt_add_dbus_interfaces(DataEngine_SRCS
                        ${KDED_DBUS_INTERFACE}
)

add_library(plasma_engine_touchpad MODULE ${DataEngine_SRCS})
target_link_libraries(plasma_engine_touchpad
                      KF5::Plasma
		      Qt::DBus
)

kcoreaddons_desktop_to_json(plasma_engine_touchpad plasma-dataengine-touchpad.desktop SERVICE_TYPES kcmodule.desktop)

install(TARGETS plasma_engine_touchpad
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine
)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/plasma-dataengine-touchpad.desktop
        DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)
install(FILES touchpad.operations
        DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services
)
