# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_touchpad\")

add_subdirectory(icon)

SET(TOUCHPAD_SRCS
    actions.cpp
    touchpadbackend.cpp
    logging.cpp
    touchpadparametersbase.cpp
)

include(backends/x11.cmake)
include(backends/kwin_wayland.cmake)

qt_add_dbus_interfaces(TOUCHPAD_SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)

qt_generate_dbus_interface(kded/kded.h org.kde.touchpad.xml)

kconfig_add_kcfg_files(TOUCHPAD_SRCS kdedsettings.kcfgc)
kconfig_add_kcfg_files(TOUCHPAD_SRCS touchpadparameters.kcfgc)

add_library(touchpad STATIC ${TOUCHPAD_SRCS} ${backend_SRCS})

set_property(TARGET touchpad PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(touchpad
    PUBLIC
    Qt::Core
    KF6::XmlGui
    PRIVATE
    ${backend_LIBS}
    Qt::DBus
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::WindowSystem
    KF6::I18n
    KF6::GlobalAccel
)
target_link_libraries(touchpad PRIVATE Qt::GuiPrivate)

add_subdirectory(kcm)
add_subdirectory(kded)
add_subdirectory(applet)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
)
