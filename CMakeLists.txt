project(touchpad-config)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(touchpadlib_SRCS touchpadbackend.cpp)
kde4_add_kcfg_files(touchpadlib_SRCS touchpadparameters.kcfgc)
kde4_add_library(ktouchpad SHARED ${touchpadlib_SRCS})
target_link_libraries(ktouchpad ${KDE4_KDECORE_LIBS})

find_package(X11 REQUIRED)
find_package(X11_XCB REQUIRED)
find_package(XCB REQUIRED)

if(NOT X11_Xinput_FOUND)
    message(FATAL_ERROR "Xinput not found")
endif()

if(NOT XCB_XINPUT_FOUND)
    message(FATAL_ERROR "xcb-xinput not found")
endif()

include_directories(
${XCB_XCB_INCLUDE_DIR}
${XCB_XINPUT_INCLUDE_DIR}
${X11_XCB_INCLUDE_DIR}
${X11_Xinput_INCLUDE_PATH}
${X11_X11_INCLUDE_PATH}
)

add_definitions(
${X11_XCB_DEFINITIONS}
${XCB_DEFINITIONS}
)

kde4_add_plugin(xlib_touchpad xlibbackend.cpp xcbatom.cpp synclientproperties.c)
target_link_libraries(xlib_touchpad
${XCB_XCB_LIBRARIES}
${X11_X11_LIB}
${X11_XCB_LIBRARIES}
${XCB_XINPUT_LIBRARIES}
${X11_Xinput_LIB}
${KDE4_KDECORE_LIBS}
ktouchpad
)

set(kcm_SRCS touchpadconfig.cpp customslider.cpp)
kde4_add_ui_files(kcm_SRCS touchpadconfig.ui)
kde4_add_plugin(kcm_touchpad ${kcm_SRCS})
target_link_libraries(kcm_touchpad ${KDE4_KDEUI_LIBS} ktouchpad)

install(TARGETS ktouchpad DESTINATION ${LIB_INSTALL_DIR})
install(FILES touchpadkcmbackend.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
install(FILES touchpad.kcfg DESTINATION ${KCFG_INSTALL_DIR})

install(TARGETS xlib_touchpad DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES xlib_touchpad.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(TARGETS kcm_touchpad DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_touchpad.desktop DESTINATION ${SERVICES_INSTALL_DIR})
