remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

add_subdirectory(libkwindevices)
add_subdirectory( keyboard )
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
add_subdirectory(kded)
add_subdirectory(runners)
add_subdirectory(spellchecking)
add_subdirectory(qtquicksettings)
add_subdirectory(touchscreen)

add_subdirectory(workspaceoptions)
add_subdirectory(workspacepresets)
if (KF6Baloo_FOUND)
   add_subdirectory(baloo)
endif()
add_subdirectory(solid_actions)

add_subdirectory(mouse)

add_subdirectory(touchpad)

if(BUILD_KCM_TABLET)
    add_subdirectory(tablet)
endif()
