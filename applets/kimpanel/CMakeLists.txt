project(kimpanel)
find_package(KDE4 REQUIRED)
#set(CMAKE_BUILD_TYPE Release)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR} agent/ dialog/ utils/)
link_directories(${KDE4_INCLUDES} ${PLASMA_INCLUDE_DIR} agent/ dialog/ utils/)

add_subdirectory(agent)
add_subdirectory(dialog)
add_subdirectory(utils)
add_subdirectory(applet)
# something very strange, it can only be built by qmake , pthread and c++ ABI related, 
# cmake seems use some dangerous compiler flags.
#add_subdirectory(scim)
