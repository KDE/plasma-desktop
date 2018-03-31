# // krazy:excludeall=copyright,license

include_directories(
    ${X11_X11_INCLUDE_PATH}
    ${X11_Xinput_INCLUDE_PATH}
    ${Evdev_INCLUDE_DIRS}
    ${XORGLIBINPUT_INCLUDE_DIRS}
)

set(backend_SRCS
    ${backend_SRCS}
    backends/x11/x11_backend.cpp
    backends/x11/x11_evdev_backend.cpp
    backends/x11/evdev_settings.cpp
    backends/x11/x11_libinput_backend.cpp
    backends/x11/x11_libinput_dummydevice.cpp
    backends/x11/libinput_settings.cpp
)

set(backend_LIBS
    ${backend_LIBS}
    KF5::WindowSystem
    Qt5::X11Extras
    ${X11_X11_LIB}
    ${X11_Xinput_LIB}
)

if (X11_Xcursor_FOUND)
    set(backend_LIBS
        ${X11_Xcursor_LIB}
        ${backend_LIBS}
    )
    include_directories(${X11_Xcursor_INCLUDE_PATH})
endif ()
