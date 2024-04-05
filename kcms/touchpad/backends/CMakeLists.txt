# // krazy:excludeall=copyright,license

target_sources(touchpad_common PUBLIC libinputcommon.cpp)

if (BUILD_KCM_TOUCHPAD_KWIN_WAYLAND)
    target_sources(touchpad_common
        PUBLIC
            kwin_wayland/kwinwaylandbackend.cpp
            kwin_wayland/kwinwaylandtouchpad.cpp
    )
endif()

if (BUILD_KCM_TOUCHPAD_X11)
    target_sources(touchpad_common
        PUBLIC
            x11/propertyinfo.cpp
            x11/xlibbackend.cpp
            x11/xlibtouchpad.cpp
            x11/xcbatom.cpp
            x11/xlibnotifications.cpp
            x11/xrecordkeyboardmonitor.cpp
            x11/libinputtouchpad.cpp
    )

    target_link_libraries(touchpad_common
        PUBLIC
            Qt::GuiPrivate
            XCB::ATOM
            XCB::RECORD
            X11::X11
            X11::Xi
            X11::XCB
            PkgConfig::XORGSERVER
            PkgConfig::XORGLIBINPUT
    )

    add_executable(kcm-touchpad-list-devices x11/listdevices.cpp)
    target_link_libraries(kcm-touchpad-list-devices
        X11::X11
        X11::Xi
    )
    install(TARGETS kcm-touchpad-list-devices
            DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
    )
endif()
