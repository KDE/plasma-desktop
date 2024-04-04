# // krazy:excludeall=copyright,license

if (BUILD_KCM_MOUSE_KWIN_WAYLAND)
    target_sources(mouse_common
        PUBLIC
            kwin_wl/kwin_wl_backend.cpp
            kwin_wl/kwin_wl_device.cpp
    )

    target_link_libraries(mouse_common PUBLIC Qt::DBus)
endif()

if (BUILD_KCM_MOUSE_X11)
    target_sources(mouse_common
        PUBLIC
            x11/x11_backend.cpp
            x11/x11_libinput_backend.cpp
            x11/x11_libinput_dummydevice.cpp
            x11/libinput_settings.cpp
    )

    target_link_libraries(mouse_common
        PUBLIC
            PkgConfig::XORGLIBINPUT
            X11::X11
            X11::Xi
            X11::Xcursor
            Qt::GuiPrivate
    )
endif()
