add_definitions(-DTRANSLATION_DOMAIN=\"kcm_tablet\")

find_package(LayerShellQt REQUIRED)

kcmutils_add_qml_kcm(kcm_tablet INSTALL_NAMESPACE "plasma/kcms/systemsettings")

ecm_qt_declare_logging_category(kcm_tablet
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
qt6_generate_wayland_protocol_client_sources(kcm_tablet FILES ${WaylandProtocols_DATADIR}/unstable/tablet/tablet-unstable-v2.xml)
ecm_qt_install_logging_categories(
    EXPORT kcm_tablet
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

target_sources(kcm_tablet PRIVATE
    kcmtablet.cpp
    tabletevents.cpp
    calibrationtool.cpp
)

target_link_libraries(kcm_tablet PRIVATE
    Qt::Quick
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::I18n
    KF6::KCMUtilsQuick

    Qt::DBus
    Qt::WaylandClient
    Wayland::Client
    kwindevices
    Canberra::Canberra
    LayerShellQt::Interface
)
if (Qt${QT_MAJOR_VERSION}_VERSION VERSION_LESS "6.5.0")
    target_link_libraries(kcm_tablet PRIVATE Qt::GuiPrivate)
endif()

