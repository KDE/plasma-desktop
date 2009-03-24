include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   )
set(kimpanelruntime_SRCS
   kimlabelgraphics.cpp
   kimlookuptable.cpp
   kimlookuptablegraphics.cpp
   kimpanellayout.cpp
   kimstatusbar.cpp
   kimstatusbargraphics.cpp

   #   kimappearancedialog.cpp
   #   kimlayoutdialog.cpp

   kimpanelagent.cpp

   kimpanelsettings.cpp

   paintutils.cpp
   )
set(kimpanelruntime_UIS
   kimpanelappearanceconfig.ui
   kimpanellayoutconfig.ui
   )
qt4_add_dbus_adaptor(kimpanelruntime_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.impanel.xml
   ${CMAKE_CURRENT_SOURCE_DIR}/kimpanelagent.h
   PanelAgent)

kde4_add_kcfg_files(kimpanelruntime_SRCS kimpanelconfig.kcfgc)
kde4_add_library(kimpanelruntime SHARED ${kimpanelruntime_SRCS})
target_link_libraries(kimpanelruntime
   ${KDE4_KNEWSTUFF2_LIBS} 
   ${KDE4_PLASMA_LIBS} 
   ${KDE4_KIO_LIBS} 
   ${KDE4_KDEUI_LIBS}
   ${QT_QTDBUS_LIBRARY}
   ${QT_QTSCRIPT_LIBRARY}
   )
set_target_properties(kimpanelruntime PROPERTIES
   VERSION 0 SOVERSION 0
   )
install(TARGETS kimpanelruntime ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kimpanelconfig.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install(FILES org.kde.impanel.xml 
   DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

set(kimpanel_applet_SRCS
   kimpanelapplet.cpp
   )
kde4_add_plugin(plasma_applet_kimpanel ${kimpanel_applet_SRCS})
target_link_libraries(plasma_applet_kimpanel 
   kimpanelruntime
   ${KDE4_PLASMA_LIBS} 
   ${KDE4_KUTILS_LIBS} 
   ${X11_LIBRARIES}
   ${X11_Xrender_LIB}
   )
install(TARGETS plasma_applet_kimpanel DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-kimpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(kimpanel_dialog_SRCS
   paneldialog.cpp
   main.cpp
   )
kde4_add_executable(kimpanel ${kimpanel_dialog_SRCS})
target_link_libraries(kimpanel 
   kimpanelruntime
   ${KDE4_PLASMA_LIBS} 
   ${KDE4_KUTILS_LIBS} 
   ${X11_LIBRARIES}
   ${X11_Xrender_LIB}
   )
install(TARGETS kimpanel ${INSTALL_TARGETS_DEFAULT_ARGS})
