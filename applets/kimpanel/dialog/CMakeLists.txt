#set(CMAKE_BUILD_TYPE Release)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

set(kimpaneldialog_SRCS
   statusbarwidget.cpp
   lookuptablewidget.cpp
   )

kde4_add_library(kimpaneldialog ${kimpaneldialog_SRCS})
target_link_libraries(kimpaneldialog 
   ${KDE4_PLASMA_LIBS} 
   kimpanelagent
   kimpanelutils
   )


set(kimpanel_SRCS
   main.cpp
   kimpanel.cpp
   )

kde4_add_executable(kimpanel ${kimpanel_SRCS})
target_link_libraries(kimpanel ${KDE4_PLASMA_LIBS} 
   kimpaneldialog
   kimpanelutils
   )

install(TARGETS kimpanel ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(FILES plasma-applet-kimpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
