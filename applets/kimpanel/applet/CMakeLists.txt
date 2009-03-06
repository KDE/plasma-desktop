project(kimpanel-applet)

# building separately or as part of kdebase ?
if(NOT KDE4_FOUND)
   find_package(KDE4 REQUIRED)

   include_directories(
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES} 
   )
endif(NOT KDE4_FOUND)

set(plasma_applet_kimpanel_SRCS
  kimpanelapplet.cpp
)

kde4_add_ui_files(plasma_applet_kimpanel_SRCS kimpanelConfig.ui)
kde4_add_ui_files(plasma_applet_kimpanel_SRCS kimpanelAdd.ui)
kde4_add_plugin(plasma_applet_kimpanel ${plasma_applet_kimpanel_SRCS})
target_link_libraries(plasma_applet_kimpanel 
   ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS}
   kimpanelagent)

install(TARGETS plasma_applet_kimpanel DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-kimpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR})
