# // krazy:excludeall=copyright,license

set(kcminput_backend_SRCS
    ${kcminput_backend_SRCS}
    backends/x11/x11mousebackend.cpp
)

set(kcminput_backend_LIBS
    Qt5::X11Extras
    ${X11_X11_LIB}
    ${X11_Xinput_LIB}
    ${kcminput_backend_LIBS}
)

include_directories(${X11_X11_INCLUDE_PATH}
                    ${X11_Xinput_INCLUDE_PATH}
                    ${Evdev_INCLUDE_DIRS}
                    ${XORGLIBINPUT_INCLUDE_DIRS})

if (X11_Xcursor_FOUND)
    set(kcminput_backend_LIBS
        ${X11_Xcursor_LIB}
        ${kcminput_backend_LIBS}
    )
    include_directories(${X11_Xcursor_INCLUDE_PATH})
endif ()


set(kapplymousetheme_SRCS
    backends/x11/kapplymousetheme.cpp)

add_executable(kapplymousetheme ${kapplymousetheme_SRCS} ${kcminput_backend_SRCS})

target_link_libraries(kapplymousetheme
    Qt5::Gui
    Qt5::DBus
    KF5::CoreAddons
    KF5::ConfigCore
    ${kcminput_backend_LIBS}
)

install(TARGETS kapplymousetheme ${INSTALL_TARGETS_DEFAULT_ARGS})
