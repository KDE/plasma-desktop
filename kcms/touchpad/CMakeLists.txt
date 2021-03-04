# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_touchpad\")

add_subdirectory(icon)

SET(SRCS
    plugins.cpp
    touchpadbackend.cpp
    logging.cpp
)

include(backends/x11.cmake)
include(backends/kwin_wayland.cmake)

set(SRCS
    ${SRCS}
    kcm/touchpadconfigcontainer.cpp
    kcm/touchpadconfigplugin.cpp
    kcm/libinput/touchpadconfiglibinput.cpp
    kcm/xlib/customslider.cpp
    kcm/xlib/sliderpair.cpp
    kcm/xlib/testarea.cpp
    kcm/xlib/testbutton.cpp
    kcm/xlib/touchpadconfigxlib.cpp
    kcm/xlib/touchpadparametersbase.cpp
    kcm/xlib/customconfigdialogmanager.cpp
)

qt5_add_dbus_interfaces(SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)

qt5_add_resources( SRCS kcm/resources.qrc )

kconfig_add_kcfg_files(SRCS kcm/xlib/touchpadparameters.kcfgc)

ki18n_wrap_ui(SRCS
            kcm/xlib/ui/pointermotion.ui
            kcm/xlib/ui/tap.ui
            kcm/xlib/ui/scroll.ui
            kcm/xlib/ui/sensitivity.ui
            kcm/xlib/ui/kded.ui
            kcm/xlib/ui/testarea.ui
)

qt5_generate_dbus_interface(kded/kded.h org.kde.touchpad.xml)

SET(SRCS
    ${SRCS}
    kded/kded.cpp
    kded/kdedactions.cpp
)

kconfig_add_kcfg_files(SRCS kded/kdedsettings.kcfgc)

add_library(kded_touchpad MODULE
            ${SRCS}
            ${backend_SRCS}
)
target_link_libraries(kded_touchpad
    ${backend_LIBS}
    KF5::KCMUtils
    KF5::Notifications
    KF5::CoreAddons
    KF5::WindowSystem
    KF5::ConfigWidgets
    KF5::DBusAddons
    KF5::Completion
    KF5::WidgetsAddons
    KF5::I18n
    KF5::Service
    KF5::NotifyConfig
    KF5::XmlGui
    KF5::GlobalAccel
    KF5::Plasma
    KF5::Declarative

    Qt::X11Extras
    Qt::QuickWidgets
)

add_subdirectory(applet)

install(FILES kcm/kcm_touchpad.desktop
        DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
)

install(FILES kded/touchpaddaemon.kcfg
        DESTINATION ${KDE_INSTALL_KCFGDIR}
)
install(FILES kded/kded_touchpad.desktop
        DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kded
        RENAME touchpad.desktop
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
)
install(FILES kded/kcm_touchpad.notifyrc
        DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR}
)

install(TARGETS kded_touchpad
        DESTINATION ${KDE_INSTALL_PLUGINDIR}
)

install(FILES kcm/xlib/touchpad.kcfg
        DESTINATION ${KDE_INSTALL_KCFGDIR}
)
