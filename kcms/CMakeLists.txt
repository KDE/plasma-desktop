remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

add_subdirectory(libkwindevices)

if(X11_Xkb_FOUND AND XCB_XKB_FOUND)
    add_subdirectory( keyboard )
endif()

add_subdirectory( access )
add_subdirectory( dateandtime )
add_subdirectory( ksplash )

add_subdirectory( landingpage )
add_subdirectory( keys )
add_subdirectory( ksmserver )
add_subdirectory( desktoppaths )

if (TARGET SDL2::SDL2)
    add_subdirectory( gamecontroller )
endif()

add_subdirectory(activities)
add_subdirectory(recentFiles)
add_subdirectory(componentchooser)
add_subdirectory(kded)
add_subdirectory(runners)
add_subdirectory(spellchecking)
add_subdirectory(qtquicksettings)
add_subdirectory(tablet)
add_subdirectory(touchscreen)

add_subdirectory(workspaceoptions)
if (KF6Baloo_FOUND)
   add_subdirectory(baloo)
endif()
add_subdirectory(solid_actions)

if (BUILD_KCM_MOUSE_KWIN_WAYLAND OR BUILD_KCM_MOUSE_X11)
    add_subdirectory(mouse)
endif()

if (BUILD_KCM_TOUCHPAD_KWIN_WAYLAND OR BUILD_KCM_TOUCHPAD_X11)
    add_subdirectory(touchpad)
endif()
