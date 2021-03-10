# // krazy:excludeall=copyright,license

SET(backend_SRCS
    ${backend_SRCS}
    backends/x11/propertyinfo.cpp
    backends/x11/xlibbackend.cpp
    backends/x11/xlibtouchpad.cpp
    backends/x11/xcbatom.cpp
    backends/x11/xlibnotifications.cpp
    backends/x11/xrecordkeyboardmonitor.cpp
)

if(SYNAPTICS_FOUND)
    list(APPEND backend_SRCS backends/x11/synapticstouchpad.cpp)
endif()

if (XORGLIBINPUT_FOUND)

    SET(backend_SRCS
        ${backend_SRCS}
        backends/libinputcommon.cpp
        backends/x11/libinputtouchpad.cpp
    )

    SET(backend_LIBS
        ${backend_LIBS}
        PkgConfig::XORGLIBINPUT
    )
endif()

SET(backend_LIBS
    ${backend_LIBS}
    XCB::ATOM
    XCB::RECORD
    X11::X11
    X11::Xi
    X11::XCB
    PkgConfig::XORGSERVER
)

if(SYNAPTICS_FOUND)
    list(APPEND backend_LIBS PkgConfig::SYNAPTICS)
endif()

add_executable(kcm-touchpad-list-devices backends/x11/listdevices.cpp)
target_link_libraries(kcm-touchpad-list-devices
                      X11::X11
                      X11::Xi
)
install(TARGETS kcm-touchpad-list-devices
        DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)
