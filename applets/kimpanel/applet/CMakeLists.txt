
# building separately or as part of kdebase ?
if(NOT KDE4_FOUND)
   find_package(KDE4 REQUIRED)

   include_directories(
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES} 
   )
endif(NOT KDE4_FOUND)

set(plasma_applet_kimpanel_SRCS ${kimpanelutils_SRCS} ${kimpanelwidgets_SRCS} ${kimpanelagent_SRCS}
  kimpanelapplet.cpp
)
qt4_add_dbus_adaptor(plasma_applet_kimpanel_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/../agent/org.kde.impanel.xml
   ${CMAKE_CURRENT_SOURCE_DIR}/../agent/kimpanelagent.h
   PanelAgent)



kde4_add_ui_files(plasma_applet_kimpanel_SRCS kimpanelconfig.ui)
#kde4_add_ui_files(plasma_applet_kimpanel_SRCS kimpanelAdd.ui)
kde4_add_plugin(plasma_applet_kimpanel ${plasma_applet_kimpanel_SRCS})
target_link_libraries(plasma_applet_kimpanel 
   ${KDE4_PLASMA_LIBS} 
   ${KDE4_KIO_LIBRARY} 
   ${KDE4_KDEUI_LIBS}
   ${QT_QTSCRIPT_LIBRARY}
   )

install(TARGETS plasma_applet_kimpanel DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-kimpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR})
