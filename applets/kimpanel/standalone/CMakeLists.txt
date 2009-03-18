#set(CMAKE_BUILD_TYPE Release)
set(kimpaneldialog_SRCS
   main.cpp
   paneldialog.cpp   
   ${kimpanelutils_SRCS} 
   ${kimpanelwidgets_SRCS} 
   ${kimpanelagent_SRCS}
   )

qt4_add_dbus_adaptor(kimpaneldialog_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/../agent/org.kde.impanel.xml
   ${CMAKE_CURRENT_SOURCE_DIR}/../agent/kimpanelagent.h
   PanelAgent)


kde4_add_executable(kimpanel ${kimpaneldialog_SRCS})
target_link_libraries(kimpanel 
   ${KDE4_PLASMA_LIBS} 
   ${QT_QTSCRIPT_LIBRARY}
   )

install(TARGETS kimpanel ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(FILES plasma-applet-kimpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
