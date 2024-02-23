target_sources(mouse_common
    PUBLIC
        backends/kwin_wl/kwin_wl_backend.cpp
        backends/kwin_wl/kwin_wl_device.cpp
)

target_link_libraries(mouse_common PUBLIC Qt::DBus)
