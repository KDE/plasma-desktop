# // krazy:excludeall=copyright,license

target_sources(touchpad_common
    PUBLIC
        libinputcommon.cpp
        kwin_wayland/kwinwaylandbackend.cpp
        kwin_wayland/kwinwaylandtouchpad.cpp
)

target_link_libraries(touchpad_common PUBLIC KF6::I18n kwindevices)
