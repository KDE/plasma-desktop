project(touchpad-config)
cmake_minimum_required(VERSION 2.8)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(touchpadlib_SRCS touchpadbackend.cpp)
kde4_add_kcfg_files(touchpadlib_SRCS touchpadparameters.kcfgc)
kde4_add_library(ktouchpad SHARED ${touchpadlib_SRCS})
target_link_libraries(ktouchpad ${KDE4_KDECORE_LIBS})

set(synclientbackend_SRCS synclientbackend.cpp)
kde4_add_plugin(synclient_touchpad ${synclientbackend_SRCS})
target_link_libraries(synclient_touchpad ${KDE4_KDECORE_LIBS} ktouchpad)

set(kcm_SRCS touchpadconfig.cpp customslider.cpp)
kde4_add_ui_files(kcm_SRCS touchpadconfig.ui)
kde4_add_plugin(kcm_touchpad ${kcm_SRCS})
target_link_libraries(kcm_touchpad ${KDE4_KDEUI_LIBS} ktouchpad)

install(TARGETS ktouchpad DESTINATION ${LIB_INSTALL_DIR})
install(FILES touchpadkcmbackend.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(TARGETS synclient_touchpad DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES synclient_touchpad.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES touchpad.kcfg DESTINATION ${KCFG_INSTALL_DIR})

install(TARGETS kcm_touchpad DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_touchpad.desktop DESTINATION ${SERVICES_INSTALL_DIR})
