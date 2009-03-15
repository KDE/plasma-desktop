#set(CMAKE_BUILD_TYPE Release)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

set(kimtestframelayout_SRCS
   framelayout.cpp
   )

kde4_add_executable(framelayout ${kimtestframelayout_SRCS})
target_link_libraries(framelayout ${KDE4_PLASMA_LIBS} 
   kimlayout
   )

set(kimtestscriptlayout_SRCS
   scriptlayout.cpp
   )

kde4_add_executable(scriptlayout ${kimtestscriptlayout_SRCS})
target_link_libraries(scriptlayout ${KDE4_PLASMA_LIBS} 
   kimlayout
   ${QT_QTSCRIPT_LIBRARY}
   )
#install(TARGETS kimpanel ${INSTALL_TARGETS_DEFAULT_ARGS})
