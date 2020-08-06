# // krazy:excludeall=copyright,license
find_package(X11 REQUIRED)
find_package(X11_XCB REQUIRED)
find_package(XCB REQUIRED COMPONENTS ATOM RECORD)
find_package(PkgConfig REQUIRED)

if(NOT X11_Xinput_FOUND)
    message(FATAL_ERROR "Xinput not found")
endif()

include_directories(${X11_Xinput_INCLUDE_PATH}
                    ${X11_X11_INCLUDE_PATH}
                    ${Synaptics_INCLUDE_DIRS}
                    ${XORGSERVER_INCLUDE_DIRS}
)

SET(backend_SRCS
    ${backend_SRCS}
    backends/x11/propertyinfo.cpp
    backends/x11/xlibbackend.cpp
    backends/x11/synapticstouchpad.cpp
    backends/x11/xlibtouchpad.cpp
    backends/x11/xcbatom.cpp
    backends/x11/xlibnotifications.cpp
    backends/x11/xrecordkeyboardmonitor.cpp
)

if (HAVE_XORGLIBINPUT)

    include_directories(${XORGLIBINPUT_INCLUDE_DIRS})

    SET(backend_SRCS
        ${backend_SRCS}
        backends/libinputcommon.cpp
        backends/x11/libinputtouchpad.cpp
    )
endif()

SET(backend_LIBS
    ${backend_LIBS}
    XCB::ATOM
    XCB::RECORD
    ${X11_X11_LIB}
    X11::XCB
    ${X11_Xinput_LIB}
)

add_executable(kcm-touchpad-list-devices backends/x11/listdevices.cpp)
target_link_libraries(kcm-touchpad-list-devices
                      ${X11_X11_LIB}
                      ${X11_Xinput_LIB}
)
install(TARGETS kcm-touchpad-list-devices
        DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)
