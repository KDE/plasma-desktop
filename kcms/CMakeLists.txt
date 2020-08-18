remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

if(X11_Xkb_FOUND AND XCB_XKB_FOUND)
    add_subdirectory( keyboard )
endif()

if (EVDEV_FOUND AND XORGLIBINPUT_FOUND AND X11_Xinput_FOUND)
    add_subdirectory( mouse )
endif()

add_subdirectory( access )
add_subdirectory( dateandtime )
add_subdirectory( autostart )
add_subdirectory( ksplash )

add_subdirectory( launch )
add_subdirectory( keys )
add_subdirectory( ksmserver )
add_subdirectory( nightcolor )
add_subdirectory( users )

add_subdirectory( hardware )
add_subdirectory( desktoppaths )

add_subdirectory(activities)
add_subdirectory(componentchooser)
add_subdirectory(kded)
add_subdirectory(formats)
add_subdirectory(notifications)
add_subdirectory(runners)
add_subdirectory(spellchecking)
add_subdirectory(qtquicksettings)

add_subdirectory(workspaceoptions)
if (KF5Baloo_FOUND)
   add_subdirectory(baloo)
endif()
add_subdirectory(solid_actions)

if (XORGSERVER_FOUND AND SYNAPTICS_FOUND AND X11_Xinput_FOUND)
    add_subdirectory(touchpad)
endif()
