#set(CMAKE_BUILD_TYPE Release)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

set(kimtestframelayout_SRCS
   framelayout.cpp
   ${kimlayout_SRCS}
   )

kde4_add_executable(framelayout ${kimtestframelayout_SRCS})
target_link_libraries(framelayout 
   ${KDE4_PLASMA_LIBS}
   ${QT_QTSCRIPT_LIBRARY}
   )

set(kimtestscriptlayout_SRCS
   scriptlayout.cpp
   ${kimlayout_SRCS}
   )

kde4_add_executable(scriptlayout ${kimtestscriptlayout_SRCS})
target_link_libraries(scriptlayout 
   ${KDE4_PLASMA_LIBS} 
   ${QT_QTSCRIPT_LIBRARY}
   )

set(kimtestlookuptablescriptlayout_SRCS
   lookuptablescriptlayout.cpp
   ${kimlayout_SRCS}
   )

kde4_add_executable(lookuptablelayout ${kimtestlookuptablescriptlayout_SRCS})
target_link_libraries(lookuptablelayout 
   ${KDE4_PLASMA_LIBS} 
   ${QT_QTSCRIPT_LIBRARY}
   )

set(kimtesttheme_SRCS
   theme.cpp
   ${kimlayout_SRCS}
   )

kde4_add_executable(theme ${kimtesttheme_SRCS})
target_link_libraries(theme
   ${KDE4_PLASMA_LIBS} 
   ${QT_QTSCRIPT_LIBRARY}
   )
