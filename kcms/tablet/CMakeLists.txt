add_definitions(-DTRANSLATION_DOMAIN=\"kcm_tablet\")

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

if (Qt6_VERSION VERSION_GREATER_EQUAL "6.8.0")
    set(private_code_option  "PRIVATE_CODE")
endif()
qt6_generate_wayland_protocol_client_sources(kcm_tablet
    ${private_code_option}
    FILES ${WaylandProtocols_DATADIR}/unstable/tablet/tablet-unstable-v2.xml
)

ecm_qt_install_logging_categories(
    EXPORT kcm_tablet
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

target_sources(kcm_tablet PRIVATE
    kcmtablet.cpp
    tabletevents.cpp
    calibrationtool.cpp
    inputsequence.cpp
)

target_link_libraries(kcm_tablet PRIVATE
    Qt::Quick
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::GuiAddons
    KF6::I18n
    KF6::KCMUtilsQuick

    Qt::DBus
    Qt::WaylandClient
    Wayland::Client
    kwindevices
    Canberra::Canberra
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
