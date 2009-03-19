set(kimpanelruntime_SRCS
   kimsvglayout.cpp
   kimframesvglayout.cpp
   kimsvgscriptitem.cpp
   kimsvgscriptlayout.cpp
   kimsvgscriptwrapper.cpp

   kimlabelgraphics.cpp
   kimlookuptable.cpp
   kimlookuptablegraphics.cpp
   kimpanellayout.cpp
   kimstatusbar.cpp
   kimstatusbargraphics.cpp

   kimpanelagent.cpp

   kimsettings.cpp

   kimtheme.cpp

   paintutils.cpp
   )
qt4_add_dbus_adaptor(kimpanelruntime_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.impanel.xml
   ${CMAKE_CURRENT_SOURCE_DIR}/kimpanelagent.h
   PanelAgent)

kde4_add_library(kimpanelruntime SHARED ${kimpanelruntime_SRCS})
target_link_libraries(kimpanelruntime
   ${KDE4_PLASMA_LIBS} 
   ${KDE4_KIO_LIBS} 
   ${KDE4_KDEUI_LIBS}
   ${QT_QTDBUS_LIBRARY}
   ${QT_QTSCRIPT_LIBRARY}
   )
install(TARGETS kimpanelruntime ${INSTALL_TARGETS_DEFAULT_ARGS})

set(kimpanel_SRCS
   main.cpp
   paneldialog.cpp
   )
kde4_add_executable(kimpanel ${kimpanel_SRCS})
target_link_libraries(kimpanel kimpanelruntime)
install(TARGETS kimpanel ${INSTALL_TARGETS_DEFAULT_ARGS})

set(kimpanel_applet_SRCS
   kimpanelapplet.cpp
   )
kde4_add_plugin(plasma_applet_kimpanel ${kimpanel_SRCS})
target_link_libraries(plasma_applet_kimpanel 
   kimpanelruntime
   ${KDE4_PLASMA_LIBS} 
   ${X11_LIBRARIES}
   ${X11_Xrender_LIB}
   )
install(TARGETS plasma_applet_kimpanel DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-kimpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR})


