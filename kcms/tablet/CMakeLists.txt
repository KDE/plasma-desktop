add_definitions(-DTRANSLATION_DOMAIN=\"kcmtablet\")

include(ECMQtDeclareLoggingCategory)
ecm_qt_declare_logging_category(common_SRCS
    HEADER
        logging.h
    IDENTIFIER
        KCM_TABLET
    CATEGORY_NAME
        kcm_tablet
    DEFAULT_SEVERITY
        Critical
    DESCRIPTION
        "KCM for tablet input"
    EXPORT
        kcm_tablet
)
ecm_qt_install_logging_categories(
    EXPORT kcm_tablet
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
qt_add_dbus_interface(common_SRCS "${KWIN_INPUTDEVICE_INTERFACE}" InputDevice_interface)

kcoreaddons_add_plugin(kcm_tablet INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file(kcm_tablet)

target_sources(kcm_tablet PRIVATE
    ${common_SRCS}
    kcmtablet.cpp
    devicesmodel.cpp
    inputdevice.cpp
)

target_link_libraries(kcm_tablet
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::I18n
    KF5::QuickAddons

    Qt::DBus
)

kpackage_install_package(package kcm_tablet kcms)
