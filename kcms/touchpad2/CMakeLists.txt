add_definitions(-DTRANSLATION_DOMAIN=\"kcm_touchpad2\")

kcmutils_add_qml_kcm(kcm_touchpad2 INSTALL_NAMESPACE "plasma/kcms/systemsettings")

ecm_qt_declare_logging_category(kcm_touchpad2
    HEADER
        logging.h
    IDENTIFIER
        KCM_TABLET
    CATEGORY_NAME
        kcm_touchpad2
    DEFAULT_SEVERITY
        Critical
    DESCRIPTION
        "KCM for tablet input"
    EXPORT
        kcm_touchpad2
)


ecm_qt_declare_logging_category(kwindevices_SRCS
    HEADER
        logging.h
    IDENTIFIER
        LIBKWINDEVICES
    CATEGORY_NAME
        org.kde.libkwindevices
    DEFAULT_SEVERITY
        Critical
    DESCRIPTION
        "Library for kwin devices"
    EXPORT
        libkwindevices
)


# qt6_generate_wayland_protocol_client_sources(kcm_touchpad2 FILES ${WaylandProtocols_DATADIR}/unstable/tablet/tablet-unstable-v2.xml)
ecm_qt_install_logging_categories(
    EXPORT kcm_touchpad2
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

qt_add_dbus_interface(kwindevices_SRCS "${KWIN_INPUTDEVICE_INTERFACE}" InputDevice_interface)

target_sources(kcm_touchpad2 PRIVATE
    kcmtablet.cpp
    ${kwindevices_SRCS}
    devicesmodel.cpp
    inputdevice.cpp
    # tabletevents.cpp
)

target_link_libraries(kcm_touchpad2
    Qt::Quick
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::I18n
    KF6::KCMUtilsQuick

    Qt::DBus
    Qt::WaylandClient
    Qt::GuiPrivate
    Wayland::Client
    # kwindevices
)

