add_definitions(-DTRANSLATION_DOMAIN=\"kcm_tablet\")

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
kcoreaddons_add_plugin(kcm_tablet INSTALL_NAMESPACE "plasma/kcms/systemsettings")
if (QT_MAJOR_VERSION EQUAL "5")
    ecm_add_qtwayland_client_protocol(common_SRCS
        PROTOCOL ${WaylandProtocols_DATADIR}/unstable/tablet/tablet-unstable-v2.xml
        BASENAME tablet-unstable-v2
    )
else()
    qt6_generate_wayland_protocol_client_sources(kcm_tablet FILES
        ${WaylandProtocols_DATADIR}/unstable/tablet/tablet-unstable-v2.xml)
endif()
ecm_qt_install_logging_categories(
    EXPORT kcm_tablet
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

kcmutils_generate_desktop_file(kcm_tablet)

target_sources(kcm_tablet PRIVATE
    ${common_SRCS}
    kcmtablet.cpp
    tabletevents.cpp
)

target_link_libraries(kcm_tablet
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::I18n
    KF6::QuickAddons

    Qt::DBus
    Qt::WaylandClient
    Qt::GuiPrivate
    Wayland::Client
    kwindevices
)

kpackage_install_package(package kcm_tablet kcms)
