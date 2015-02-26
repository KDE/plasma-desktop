# // krazy:excludeall=copyright,license
find_package(X11 REQUIRED)
find_package(X11_XCB REQUIRED)
find_package(XCB REQUIRED)
find_package(PkgConfig REQUIRED)

if(NOT X11_Xinput_FOUND)
    message(FATAL_ERROR "Xinput not found")
endif()

include_directories(${XCB_INCLUDE_DIR}
                    ${X11_XCB_INCLUDE_DIR}
                    ${X11_Xinput_INCLUDE_PATH}
                    ${X11_X11_INCLUDE_PATH}
                    ${Synaptics_INCLUDE_DIRS}
                    ${XORG_INCLUDE_DIRS}
)

add_definitions(${X11_XCB_DEFINITIONS} ${XCB_DEFINITIONS})

SET(backend_SRCS
    ${backend_SRCS}
    backends/x11/synclientproperties.c
    backends/x11/libinputproperties.c
    backends/x11/xcbatom.cpp
    backends/x11/xlibbackend.cpp
    backends/x11/xlibnotifications.cpp
    backends/x11/xrecordkeyboardmonitor.cpp
)

SET(backend_LIBS
    ${backend_LIBS}
    ${XCB_LIBRARIES}
    ${X11_X11_LIB}
    ${X11_XCB_LIBRARIES}
    ${X11_Xinput_LIB}
)

add_executable(kcm-touchpad-list-devices backends/x11/listdevices.cpp)
target_link_libraries(kcm-touchpad-list-devices
                      ${X11_X11_LIB}
                      ${X11_Xinput_LIB}
)
install(TARGETS kcm-touchpad-list-devices
        DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS}
)
