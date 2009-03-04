project(kimpanel)
#set(CMAKE_BUILD_TYPE Release)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

set(kimpanel_SRCS
   main.cpp
   kimpanel.cpp
   panelagent.cpp
   statusbarwidget.cpp
   lookuptablewidget.cpp
   )
qt4_add_dbus_adaptor(kimpanel_SRCS 
   org.kde.impanel.xml 
   panelagent.h
   PanelAgent)

kde4_add_executable(kimpanel ${kimpanel_SRCS})
target_link_libraries(kimpanel ${KDE4_PLASMA_LIBS})

install(TARGETS kimpanel ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(FILES plasma-applet-kimpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
