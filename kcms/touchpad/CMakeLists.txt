# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_touchpad\")

if (BUILD_KCM_TOUCHPAD_X11)
    if (NOT XORGLIBINPUT_FOUND)
        list(APPEND MISSING_DEPS "xorg-libinput")
    endif()
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
endif()

add_subdirectory(icon)

qt_generate_dbus_interface(kded/kded.h org.kde.touchpad.xml)
qt_add_dbus_interfaces(TOUCHPAD_DBUS_SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
)

add_library(touchpad_common STATIC
    actions.cpp
    touchpadbackend.cpp
    logging.cpp
    touchpadparametersbase.cpp
    ${TOUCHPAD_DBUS_SRCS}
)
set_property(TARGET touchpad_common PROPERTY POSITION_INDEPENDENT_CODE ON)

kconfig_add_kcfg_files(touchpad_common kdedsettings.kcfgc)
kconfig_add_kcfg_files(touchpad_common touchpadparameters.kcfgc)

target_link_libraries(touchpad_common
    PUBLIC
    Qt::Core
    KF6::XmlGui
    PRIVATE
    KF6::ConfigCore
    KF6::WindowSystem
    KF6::I18n
    KF6::GlobalAccel
)

add_subdirectory(backends)
add_subdirectory(kded)

kcmutils_add_qml_kcm(kcm_touchpad)
target_sources(kcm_touchpad PRIVATE kcm.cpp)

target_link_libraries(kcm_touchpad
    PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Quick
    Qt::QuickWidgets
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtils
    Plasma::Plasma
    KF6::WindowSystem
    touchpad_common
)

add_custom_command(TARGET kcm_touchpad POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink "../kcms/systemsettings/kcm_touchpad.so" "kcm_touchpad_init.so")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_touchpad_init.so DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcminit)
