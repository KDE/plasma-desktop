# // krazy:excludeall=copyright,license

target_sources(touchpad_common PUBLIC libinputcommon.cpp)

if (BUILD_KCM_TOUCHPAD_KWIN_WAYLAND)
    target_sources(touchpad_common
        PUBLIC
            kwin_wayland/kwinwaylandbackend.cpp
            kwin_wayland/kwinwaylandtouchpad.cpp
    )
    target_link_libraries(touchpad_common PUBLIC KF6::I18n kwindevices)
endif()
