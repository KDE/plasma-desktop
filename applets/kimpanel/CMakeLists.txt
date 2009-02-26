find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

set(kimpanel_applet_SRCS
   kimpanel.cpp
   panelagent.cpp
   )
qt4_add_dbus_adaptor(kimpanel_applet_SRCS 
   org.kde.impanel.xml 
   panelagent.h
   PanelAgent)

kde4_add_plugin(plasma_applet_kimpanel ${kimpanel_applet_SRCS})
target_link_libraries(plasma_applet_kimpanel ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_applet_kimpanel DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-kimpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
