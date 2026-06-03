# // krazy:excludeall=copyright,license

if (BUILD_KCM_MOUSE_KWIN_WAYLAND)
    target_sources(kcm_mouse
        PUBLIC
            kwin_wl/kwin_wl_backend.cpp
            kwin_wl/kwin_wl_device.cpp
    )

    target_link_libraries(kcm_mouse PUBLIC Qt::DBus PRIVATE kwindevices)
endif()
