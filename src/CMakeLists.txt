configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/version.h"
)

SET(SRCS
    plugins.cpp
    touchpadparameters.cpp
    touchpadbackend.cpp
)

kde4_add_kcfg_files(SRCS touchpadparametersbase.kcfgc)

include(backends/x11.cmake)

set(SRCS
    ${SRCS}
    kcm/customslider.cpp
    kcm/sliderpair.cpp
    kcm/testarea.cpp
    kcm/testbutton.cpp
    kcm/touchpadconfig.cpp
)

qt4_add_dbus_interfaces(SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)

kde4_add_ui_files(SRCS
                  kcm/ui/pointermotion.ui
                  kcm/ui/tap.ui
                  kcm/ui/scroll.ui
                  kcm/ui/sensitivity.ui
                  kcm/ui/testarea.ui
)

qt4_generate_dbus_interface(kded/kded.h org.kde.touchpad.xml)

SET(SRCS
    ${SRCS}
    kded/kded.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)

kde4_add_kcfg_files(SRCS kded/kdedsettings.kcfgc)

kde4_add_plugin(kded_touchpad
                ${SRCS}
                ${backend_SRCS}
)
target_link_libraries(kded_touchpad
                      ${backend_LIBS}
                      ${KDE4_KDEUI_LIBS}
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
        DESTINATION ${DATA_INSTALL_DIR}/kcm_touchpad
)

install(TARGETS kded_touchpad
        DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(FILES touchpad.kcfg
        DESTINATION ${KCFG_INSTALL_DIR}
)
