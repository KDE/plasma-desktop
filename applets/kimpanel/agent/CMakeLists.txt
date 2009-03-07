#set(CMAKE_BUILD_TYPE Release)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

set(kimpanelagent_SRCS
   kimpanelagent.cpp
   )
qt4_add_dbus_adaptor(kimpanelagent_SRCS 
   org.kde.impanel.xml 
   kimpanelagent.h
   PanelAgent)

kde4_add_library(kimpanelagent ${kimpanelagent_SRCS})
target_link_libraries(kimpanelagent ${KDE4_PLASMA_LIBS})

#install(TARGETS kimpanel ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(FILES kimpanelagent.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/kimpanel)
