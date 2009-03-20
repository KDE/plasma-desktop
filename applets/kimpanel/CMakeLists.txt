project(kimpanel)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(X11 REQUIRED)
if (NOT X11_Xrender_FOUND)
   message(FATAL "Xrender library not found")
endif (NOT X11_Xrender_FOUND)

find_package(KDE4 REQUIRED)

#set(CMAKE_BUILD_TYPE Release)

include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} 
   ${PLASMA_INCLUDE_DIR} 
   ${X11_INCLUDE_DIR}
   ${X11_Xrender_INCLUDE_PATH}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARAY_DIR}
   src/ 
   )
link_directories(${KDE4_INCLUDES} 
   ${PLASMA_INCLUDE_DIR} 
   src/
   )

add_subdirectory(src)
add_subdirectory(themes)
add_subdirectory(tests)

add_subdirectory(backend)

