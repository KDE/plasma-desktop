project(kimpanel)
find_package(KDE4 REQUIRED)
#set(CMAKE_BUILD_TYPE Release)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR})

add_subdirectory(src)
#add_subdirectory(scim)
