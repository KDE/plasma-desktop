configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/version.h"
)

SET(SRCS
    plugins.cpp
    touchpadbackend.cpp
)

include(backends/x11.cmake)

set(SRCS
    ${SRCS}
    kcm/customslider.cpp
    kcm/sliderpair.cpp
    kcm/testarea.cpp
    kcm/testbutton.cpp
    kcm/touchpadconfig.cpp
    kcm/touchpadparametersbase.cpp
    kcm/customconfigdialogmanager.cpp
)

qt5_add_dbus_interfaces(SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)

kconfig_add_kcfg_files(SRCS kcm/touchpadparameters.kcfgc)

qt5_wrap_ui(SRCS
            kcm/ui/pointermotion.ui
            kcm/ui/tap.ui
            kcm/ui/scroll.ui
            kcm/ui/sensitivity.ui
            kcm/ui/kded.ui
            kcm/ui/testarea.ui
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
                      
		      Qt5::X11Extras
)

add_subdirectory(applet)

install(FILES kcm/kcm_touchpad.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
)

install(FILES kded/touchpaddaemon.kcfg
        DESTINATION ${KCFG_INSTALL_DIR}
)
install(FILES kded/kded_touchpad.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}/kded
        RENAME touchpad.desktop
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
        DESTINATION ${DBUS_INTERFACES_INSTALL_DIR}
)
install(FILES kded/kcm_touchpad.notifyrc
        DESTINATION ${KNOTIFYRC_INSTALL_DIR}
)

install(TARGETS kded_touchpad
        DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(FILES kcm/touchpad.kcfg
        DESTINATION ${KCFG_INSTALL_DIR}
)
