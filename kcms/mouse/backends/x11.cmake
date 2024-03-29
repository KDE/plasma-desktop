# // krazy:excludeall=copyright,license

set(backend_SRCS
    ${backend_SRCS}
    backends/x11/x11_backend.cpp
    backends/x11/x11_libinput_backend.cpp
    backends/x11/x11_libinput_dummydevice.cpp
    backends/x11/libinput_settings.cpp
)

set(backend_LIBS
    ${backend_LIBS}
    PkgConfig::XORGLIBINPUT
    PkgConfig::EVDEV
    X11::X11
    X11::Xi
    X11::Xcursor
)
list(APPEND backend_LIBS Qt::GuiPrivate)
