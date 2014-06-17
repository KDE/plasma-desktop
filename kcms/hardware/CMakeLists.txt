if (CMAKE_SYSTEM_NAME MATCHES Linux)
   # this one doesn't seem to be very portable, Alex
   add_subdirectory( joystick )
endif ()
