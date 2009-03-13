#set(CMAKE_BUILD_TYPE Release)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

set(kimpaneldialog_SRCS
   main.cpp
   paneldialog.cpp
   )

kde4_add_executable(kimpanel ${kimpaneldialog_SRCS})
target_link_libraries(kimpanel ${KDE4_PLASMA_LIBS} 
   kimpanelagent
   kimpanelwidgets
   kimpanelutils
   )

install(TARGETS kimpanel ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(FILES plasma-applet-kimpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
