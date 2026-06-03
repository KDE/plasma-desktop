# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_touchpad\")

add_subdirectory(icon)

add_library(touchpad_common STATIC
    touchpadbackend.cpp
)
set_property(TARGET touchpad_common PROPERTY POSITION_INDEPENDENT_CODE ON)

ecm_qt_declare_logging_category(touchpad_common
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

target_link_libraries(touchpad_common
    PUBLIC
    Qt::Core
    KF6::XmlGui
    PRIVATE
    KF6::ConfigCore
    KF6::WindowSystem
    KF6::I18n
)

add_subdirectory(backends)

kcmutils_add_qml_kcm(kcm_touchpad)
target_sources(kcm_touchpad PRIVATE kcm.cpp touchpadmoduledata.cpp)

target_link_libraries(kcm_touchpad
    PRIVATE
    Qt::Core
    Qt::Widgets
    Qt::Quick
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtilsQuick
    Plasma::Plasma
    KF6::WindowSystem
    touchpad_common
)

add_custom_command(TARGET kcm_touchpad POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink "../kcms/systemsettings/kcm_touchpad.so" "kcm_touchpad_init.so")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_touchpad_init.so DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcminit)

install(FILES org.kde.touchpadshortcuts.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kglobalaccel)

ecm_qt_install_logging_categories(
    EXPORT kcm_touchpad
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
