kcoreaddons_add_plugin(kded_touchpad SOURCES kded.cpp INSTALL_NAMESPACE "kf5/kded")

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
