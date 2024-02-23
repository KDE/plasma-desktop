# // krazy:excludeall=copyright,license

target_sources(mouse_common
    PUBLIC
        backends/x11/x11_backend.cpp
        backends/x11/x11_libinput_backend.cpp
        backends/x11/x11_libinput_dummydevice.cpp
        backends/x11/libinput_settings.cpp
)

target_link_libraries(mouse_common
    PUBLIC
        PkgConfig::XORGLIBINPUT
        PkgConfig::EVDEV
        X11::X11
        X11::Xi
        X11::Xcursor
        Qt::GuiPrivate
)
