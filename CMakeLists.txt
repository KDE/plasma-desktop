project(touchpad-config)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(TOUCHPAD_KCM_VERSION "0.1")
configure_file(
"${PROJECT_SOURCE_DIR}/version.h.cmake"
"${PROJECT_BINARY_DIR}/version.h")
configure_file(
"${PROJECT_SOURCE_DIR}/touchpad-applet.desktop.cmake"
"${PROJECT_BINARY_DIR}/touchpad-applet.desktop")

find_package(X11 REQUIRED)
find_package(X11_XCB REQUIRED)
find_package(XCB REQUIRED)
find_package(PkgConfig REQUIRED)

if(NOT X11_Xinput_FOUND)
    message(FATAL_ERROR "Xinput not found")
endif()

pkg_search_module(SYNAPTICS REQUIRED xorg-synaptics)
pkg_search_module(XORG REQUIRED xorg-server)

include_directories(
${XCB_INCLUDE_DIR}
${X11_XCB_INCLUDE_DIR}
${X11_Xinput_INCLUDE_PATH}
${X11_X11_INCLUDE_PATH}
${SYNAPTICS_INCLUDE_DIRS}
${XORG_INCLUDE_DIRS}
)

add_definitions(
${X11_XCB_DEFINITIONS}
${XCB_DEFINITIONS}
)

qt4_generate_dbus_interface(kded.h org.kde.touchpad.xml)

set(SRCS
touchpadconfig.cpp
testarea.cpp
testbutton.cpp
customslider.cpp
sliderpair.cpp
plugins.cpp
touchpadparameters.cpp
kded.cpp

touchpadbackend.cpp
#Xlib backend
xlibbackend.cpp
xcbatom.cpp
xlibnotifications.cpp
xrecordkeyboardmonitor.cpp
synclientproperties.c

${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)

set(Applet_SRCS
trayicon.cpp
)

kde4_add_kcfg_files(SRCS touchpadparametersbase.kcfgc)
kde4_add_kcfg_files(SRCS kdedsettings.kcfgc)
kde4_add_ui_files(SRCS
pointermotion.ui
tap.ui
scroll.ui
sensitivity.ui
testarea.ui)
kde4_add_plugin(kded_touchpad ${SRCS})

target_link_libraries(kded_touchpad
${XCB_LIBRARIES}
${X11_X11_LIB}
${X11_XCB_LIBRARIES}
${X11_Xinput_LIB}
${KDE4_KDEUI_LIBS}
)

qt4_add_dbus_interfaces(Applet_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml)
kde4_add_plugin(touchpad_applet ${Applet_SRCS})
target_link_libraries(touchpad_applet
${KDE4_PLASMA_LIBS}
${KDE4_KDEUI_LIBS}
${KDE4_KUTILS_LIBS}
)

add_executable(kde-touchpad-list-input-devices listdevices.cpp)
target_link_libraries(kde-touchpad-list-input-devices
${X11_X11_LIB}
${X11_Xinput_LIB}
)
install(TARGETS kde-touchpad-list-input-devices
DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES touchpad.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES touchpaddaemon.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(TARGETS kded_touchpad DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_touchpad.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kded_touchpad.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded
RENAME touchpad.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
install(FILES kcm_touchpad.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kcm_touchpad)

install(TARGETS touchpad_applet DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/touchpad-applet.desktop
DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES touchpad.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/icons)

kde4_install_icons(${ICON_INSTALL_DIR})
kde4_update_iconcache()
