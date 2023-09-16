add_definitions(-DTRANSLATION_DOMAIN=\"kcm_touchscreen\")

kcmutils_add_qml_kcm(kcm_touchscreen SOURCES kcmtouchscreen.cpp)

ecm_qt_declare_logging_category(kcm_touchscreen
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

ecm_qt_install_logging_categories(
    EXPORT kcm_touchscreen
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
target_link_libraries(kcm_touchscreen PRIVATE
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::I18n
    KF6::KCMUtilsQuick

    Qt::Gui
    Qt::DBus
    kwindevices
)
