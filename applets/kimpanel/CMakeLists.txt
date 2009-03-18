project(kimpanel)
find_package(KDE4 REQUIRED)
#set(CMAKE_BUILD_TYPE Release)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} 
   ${PLASMA_INCLUDE_DIR} 
   agent/ 
   widgets/ 
   utils/ 
   applet/ 
   layout/)
link_directories(${KDE4_INCLUDES} 
   ${PLASMA_INCLUDE_DIR} 
   lagent/ 
   widgets/ 
   utils/ 
   layout/)

set(kimlayout_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/kimsvglayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/kimframesvglayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/kimsvgscriptitem.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/kimsvgscriptwrapper.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/kimsvgscriptlayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/kimtheme.cpp
   )

set(kimpanelwidgets_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets/kimpanellayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets/kimstatusbar.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets/kimstatusbargraphics.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets/kimlookuptable.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets/kimlookuptablegraphics.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgets/kimlabelgraphics.cpp
   ${kimlayout_SRCS}
   )
set(kimpanelutils_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/utils/paintutils.cpp
   )

set(kimpanelagent_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/agent/kimpanelagent.cpp
   )

set(THEME_INSTALL_DIR
   ${DATA_INSTALL_DIR}/kimpanel/themes
   )

add_subdirectory(standalone)
add_subdirectory(applet)
add_subdirectory(themes)
add_subdirectory(tests)

