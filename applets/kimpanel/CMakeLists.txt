
include_directories(${KDE4_INCLUDES} 
   ${PLASMA_INCLUDE_DIR} 
   ${X11_INCLUDE_DIR}
   ${X11_Xrender_INCLUDE_PATH}
   )
link_directories(${KDE4_LIB_DIR})

add_subdirectory(src)
add_subdirectory(tests)

add_subdirectory(backend)

