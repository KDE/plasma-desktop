# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_touchpad\")

add_subdirectory(icon)

kcmutils_add_qml_kcm(kcm_touchpad)

ecm_qt_declare_logging_category(kcm_touchpad
    HEADER
        logging.h
    IDENTIFIER
        KCM_TOUCHPAD
    CATEGORY_NAME
        kcm_touchpad
    DESCRIPTION
        "KCM for touchpads"
    EXPORT
        kcm_touchpad
)

target_sources(kcm_touchpad PRIVATE kcm.cpp touchpadmoduledata.cpp kwinwaylandbackend.cpp kwinwaylandtouchpad.cpp)

target_link_libraries(kcm_touchpad
    PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Quick
    Qt::DBus
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtilsQuick
    Plasma::Plasma
    KF6::WindowSystem
    kwindevices
)

install(FILES org.kde.touchpadshortcuts.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kglobalaccel)

ecm_qt_install_logging_categories(
    EXPORT kcm_touchpad
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
