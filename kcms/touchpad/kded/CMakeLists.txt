kcoreaddons_add_plugin(kded_touchpad SOURCES kded.cpp INSTALL_NAMESPACE "kf5/kded")

target_link_libraries(kded_touchpad
    PRIVATE
    Qt::DBus
    KF5::DBusAddons
    KF5::I18n
    KF5::Notifications
    touchpad
)

install(FILES kcm_touchpad.notifyrc
        DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR}
)
