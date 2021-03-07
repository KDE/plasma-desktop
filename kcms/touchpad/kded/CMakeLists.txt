add_library(kded_touchpad MODULE kded.cpp)

kcoreaddons_desktop_to_json(kded_touchpad kded_touchpad.desktop)

target_link_libraries(kded_touchpad
    PRIVATE
    Qt::DBus
    KF5::DBusAddons
    KF5::I18n
    KF5::Notifications
    touchpad
)

install(FILES kcm_touchpad.notifyrc
        DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR}
)

install(TARGETS kded_touchpad
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded
)
