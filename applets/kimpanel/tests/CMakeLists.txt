#set(CMAKE_BUILD_TYPE Release)
#find_package(KDE4 REQUIRED)
#include(KDE4Defaults)
#include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_BINARY_DIR}/../src )

set(kimtestframelayout_SRCS framelayout.cpp)
kde4_add_executable(framelayout ${kimtestframelayout_SRCS})
target_link_libraries(framelayout kimpanelruntime)

set(kimtestscriptlayout_SRCS scriptlayout.cpp)
kde4_add_executable(scriptlayout ${kimtestscriptlayout_SRCS})
target_link_libraries(scriptlayout kimpanelruntime)

set(kimtestlookuptablescriptlayout_SRCS lookuptablescriptlayout.cpp)
kde4_add_executable(lookuptablelayout ${kimtestlookuptablescriptlayout_SRCS})
target_link_libraries(lookuptablelayout kimpanelruntime) 

set(kimtesttheme_SRCS theme.cpp)
kde4_add_executable(theme ${kimtesttheme_SRCS})
target_link_libraries(theme kimpanelruntime)

set(kimtestsettings_SRCS settings.cpp)
kde4_add_executable(settings ${kimtestsettings_SRCS})
target_link_libraries(settings kimpanelruntime)
