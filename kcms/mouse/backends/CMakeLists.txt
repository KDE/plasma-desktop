# // krazy:excludeall=copyright,license

target_sources(kcm_mouse
    PUBLIC
        kwin_wl/kwin_wl_device.cpp
)

target_link_libraries(kcm_mouse PUBLIC Qt::DBus PRIVATE kwindevices)
