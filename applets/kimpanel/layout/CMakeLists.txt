set(kimlayout_SRCS
   kimsvglayout.cpp
   kimframesvglayout.cpp
   kimsvgscriptitem.cpp
   kimsvgscriptwrapper.cpp
   kimsvgscriptlayout.cpp
   )

kde4_add_library(kimlayout ${kimlayout_SRCS})
target_link_libraries(kimlayout
   ${KDE4_PLASMA_LIBS} 
   )

