project(kcms)

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

find_package(Freetype)
set_package_properties(Freetype PROPERTIES DESCRIPTION "A font rendering engine"
                       URL "http://www.freetype.org"
                       TYPE OPTIONAL
                       PURPOSE "Needed to build kfontinst, a simple font installer."
                      )


set(libkxftconfig_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/fonts/kxftconfig.cpp )

if(X11_Xkb_FOUND AND XCB_XKB_FOUND)
    add_subdirectory( keyboard )
endif()

add_subdirectory( bell )

if (LIBUSB_FOUND)
    add_subdirectory( input )
endif()

add_subdirectory( access )
add_subdirectory( dateandtime )
add_subdirectory( autostart )
add_subdirectory( ksplash )

add_subdirectory( launch )
add_subdirectory( colors )
add_subdirectory( krdb )
add_subdirectory( style )
add_subdirectory( desktoptheme )
add_subdirectory( standard_actions )
add_subdirectory( keys )
add_subdirectory( ksmserver )
add_subdirectory( lookandfeel )

add_subdirectory( hardware )
add_subdirectory( desktoppaths )

add_subdirectory(componentchooser)
add_subdirectory(emoticons)
add_subdirectory(icons)
add_subdirectory(kded)
add_subdirectory(knotify)
add_subdirectory(formats)
add_subdirectory(translations)
add_subdirectory(spellchecking)
add_subdirectory(phonon)
add_subdirectory(runners)
add_subdirectory(useraccount)
add_subdirectory(workspaceoptions)
add_subdirectory(baloo)

if( FREETYPE_FOUND )
   if(FONTCONFIG_FOUND )
       add_subdirectory( fonts )
   endif()
endif()
if(FONTCONFIG_FOUND AND FREETYPE_FOUND)
    add_subdirectory( kfontinst )
endif()
