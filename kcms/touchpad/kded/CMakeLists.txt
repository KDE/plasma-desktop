kcoreaddons_add_plugin(kded_touchpad SOURCES kded.cpp INSTALL_NAMESPACE kf6/kded)

target_link_libraries(kded_touchpad
    PRIVATE
    Qt::DBus
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::I18n
    KF6::Notifications
    touchpad_common
)

install(FILES kcm_touchpad.notifyrc
        DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR}
)
