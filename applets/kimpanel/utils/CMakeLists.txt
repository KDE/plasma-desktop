#set(CMAKE_BUILD_TYPE Release)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

set(kimpanelutils_SRCS
   paintutils.cpp
   )

kde4_add_library(kimpanelutils ${kimpanelutils_SRCS})
target_link_libraries(kimpanelutils ${KDE4_PLASMA_LIBS})

#install(TARGETS kimpanel ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(FILES kimpanelutils.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/kimpanel)
