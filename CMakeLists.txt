project(touchpad-config)
cmake_minimum_required(VERSION 2.8)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(kcm_SRCS touchpadconfig.cpp touchpadbackend.cpp)
kde4_add_ui_files(kcm_SRCS touchpadconfig.ui)
kde4_add_kcfg_files(kcm_SRCS touchpadparameters.kcfgc)

kde4_add_plugin(kcm_touchpad ${kcm_SRCS})
target_link_libraries(kcm_touchpad ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_touchpad DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_touchpad.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES touchpad.kcfg DESTINATION ${KCFG_INSTALL_DIR})
