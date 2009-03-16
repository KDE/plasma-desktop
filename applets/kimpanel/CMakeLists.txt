project(kimpanel)
find_package(KDE4 REQUIRED)
#set(CMAKE_BUILD_TYPE Release)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR} agent/ widgets/ utils/ applet/ layout/)
link_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR} agent/ widgets/ utils/ layout/)

set(kimpanelwidgets_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets/kimpanellayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets/kimstatusbar.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets/kimstatusbargraphics.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets/kimlookuptable.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets/kimlookuptablegraphics.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets/kimlabelgraphics.cpp
   )
set(kimpanelutils_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/paintutils.cpp
   )

set(kimpanelagent_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/agent/kimpanelagent.cpp
   )


add_subdirectory(standalone)
add_subdirectory(applet)
add_subdirectory(layout)
add_subdirectory(tests)
