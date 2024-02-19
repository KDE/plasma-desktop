# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_touchpad\")

add_subdirectory(icon)

set(TOUCHPAD_SRCS
    actions.cpp
    touchpadbackend.cpp
    logging.cpp
    touchpadparametersbase.cpp
)

if (BUILD_KCM_TOUCHPAD_X11)
    if (NOT X11_XCB_FOUND)
        list(APPEND MISSING_DEPS "X11_XCB")
    endif()
    if (NOT X11_Xi_FOUND)
        list(APPEND MISSING_DEPS "X11_Xi")
    endif()
    if (NOT XORGSERVER_FOUND)
        list(APPEND MISSING_DEPS "xorg-server")
    endif()
    if (MISSING_DEPS)
        message(FATAL_ERROR "Missing X11 dependencies for kcm_touchpad: ${MISSING_DEPS}. Install dependencies or set CMake option -DBUILD_KCM_TOUCHPAD_X11=OFF.")
    endif()
    include(backends/x11.cmake)
endif()

if (BUILD_KCM_TOUCHPAD_KWIN_WAYLAND)
    include(backends/kwin_wayland.cmake)
endif()

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

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
)
