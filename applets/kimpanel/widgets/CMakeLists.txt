#set(CMAKE_BUILD_TYPE Release)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

set(kimpanelwidgets_SRCS
   kimpanellayout.cpp
   kimstatusbar.cpp
   kimstatusbargraphics.cpp
   kimlookuptable.cpp
   kimlookuptablegraphics.cpp
   kimlabelgraphics.cpp
   )

kde4_add_library(kimpanelwidgets ${kimpanelwidgets_SRCS})
target_link_libraries(kimpanelwidgets 
   ${KDE4_PLASMA_LIBS} 
   kimpanelagent
   kimpanelutils
   )

