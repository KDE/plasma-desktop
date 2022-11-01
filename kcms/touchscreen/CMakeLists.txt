add_definitions(-DTRANSLATION_DOMAIN=\"kcm_touchscreen\")

include(ECMQtDeclareLoggingCategory)
ecm_qt_declare_logging_category(common_SRCS
    HEADER
        logging.h
    IDENTIFIER
        KCM_TOUCHSCREEN
    CATEGORY_NAME
        kcm_touchscreen
    DEFAULT_SEVERITY
        Critical
    DESCRIPTION
        "KCM for touchscreen input"
    EXPORT
        kcm_touchscreen
)
kcoreaddons_add_plugin(kcm_touchscreen INSTALL_NAMESPACE "plasma/kcms/systemsettings")

ecm_qt_install_logging_categories(
    EXPORT kcm_touchscreen
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
qt_add_dbus_interface(common_SRCS "${KWIN_INPUTDEVICE_INTERFACE}" InputDevice_interface)

kcmutils_generate_desktop_file(kcm_touchscreen)

target_sources(kcm_touchscreen PRIVATE
    ${common_SRCS}
    kcmtouchscreen.cpp
    devicesmodel.cpp
    inputdevice.cpp
)

target_link_libraries(kcm_touchscreen
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::I18n
    KF5::QuickAddons

    Qt::DBus
)

kpackage_install_package(package kcm_touchscreen kcms)
