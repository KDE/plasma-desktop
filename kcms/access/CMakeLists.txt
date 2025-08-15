# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmaccess\")

kcmutils_add_qml_kcm(kcm_access SOURCES kcmaccess.cpp)

add_library(kaccess_settings STATIC)
set_property(TARGET kaccess_settings PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(kaccess_settings PUBLIC Qt::Core KF6::ConfigGui)

kconfig_target_kcfg_file(kaccess_settings
    GENERATE_MOC
    FILE kcmaccessibilitybell.kcfg
    HEADER kcmaccessibilitybell.h
    CLASS_NAME BellSettings
    MUTATORS DEFAULT_VALUE_GETTERS GENERATE_PROPERTIES PARENT_IN_CONSTRUCTOR NOTIFIERS
)

kconfig_target_kcfg_file(kaccess_settings
    GENERATE_MOC
    FILE kcmaccessibilitykeyboard.kcfg
    HEADER kcmaccessibilitykeyboard.h
    CLASS_NAME KeyboardSettings
    MUTATORS DEFAULT_VALUE_GETTERS GENERATE_PROPERTIES PARENT_IN_CONSTRUCTOR NOTIFIERS
)

kconfig_target_kcfg_file(kaccess_settings
    GENERATE_MOC
    FILE kcmaccessibilitykeyboardfilters.kcfg
    HEADER kcmaccessibilitykeyboardfilters.h
    CLASS_NAME KeyboardFiltersSettings
    MUTATORS DEFAULT_VALUE_GETTERS GENERATE_PROPERTIES PARENT_IN_CONSTRUCTOR NOTIFIERS
)

kconfig_target_kcfg_file(kaccess_settings
    GENERATE_MOC
    FILE kcmaccessibilitymouse.kcfg
    HEADER kcmaccessibilitymouse.h
    CLASS_NAME MouseSettings
    MUTATORS DEFAULT_VALUE_GETTERS GENERATE_PROPERTIES PARENT_IN_CONSTRUCTOR NOTIFIERS
)

kconfig_target_kcfg_file(kaccess_settings
    GENERATE_MOC
    FILE kcmaccessibilityshakecursor.kcfg
    HEADER kcmaccessibilityshakecursor.h
    CLASS_NAME ShakeCursorSettings
    MUTATORS DEFAULT_VALUE_GETTERS GENERATE_PROPERTIES PARENT_IN_CONSTRUCTOR NOTIFIERS
)

kconfig_target_kcfg_file(kaccess_settings
    GENERATE_MOC
    FILE kcmaccessibilitycolorblindnesscorrection.kcfg
    HEADER kcmaccessibilitycolorblindnesscorrection.h
    CLASS_NAME ColorblindnessCorrectionSettings
    MUTATORS DEFAULT_VALUE_GETTERS GENERATE_PROPERTIES PARENT_IN_CONSTRUCTOR NOTIFIERS
)

kconfig_target_kcfg_file(kaccess_settings
    GENERATE_MOC
    FILE kcmaccessibilityinvert.kcfg
    HEADER kcmaccessibilityinvert.h
    CLASS_NAME InvertSettings
    MUTATORS DEFAULT_VALUE_GETTERS GENERATE_PROPERTIES PARENT_IN_CONSTRUCTOR NOTIFIERS
)

kconfig_target_kcfg_file(kaccess_settings
    GENERATE_MOC
    FILE kcmaccessibilityzoommagnifier.kcfg
    HEADER kcmaccessibilityzoommagnifier.h
    CLASS_NAME ZoomMagnifierSettings
    MUTATORS DEFAULT_VALUE_GETTERS GENERATE_PROPERTIES PARENT_IN_CONSTRUCTOR NOTIFIERS
)

kcmutils_generate_module_data(kcm_access
    MODULE_DATA_HEADER kcmaccessibilitydata.h
    MODULE_DATA_CLASS_NAME AccessibilityData
    SETTINGS_HEADERS kcmaccessibilitybell.h kcmaccessibilitykeyboard.h kcmaccessibilitykeyboardfilters.h kcmaccessibilitymouse.h kcmaccessibilityshakecursor.h kcmaccessibilitycolorblindnesscorrection.h kcmaccessibilityinvert.h kcmaccessibilityzoommagnifier.h
    SETTINGS_CLASSES BellSettings KeyboardSettings KeyboardFiltersSettings MouseSettings ShakeCursorSettings ColorblindnessCorrectionSettings InvertSettings ZoomMagnifierSettings
)

ecm_qt_declare_logging_category(kcm_access
    HEADER
        logging.h
    IDENTIFIER
        KCM_ACCESS
    CATEGORY_NAME
        kcm_access
    DESCRIPTION
        "Accessibility KCM"
    EXPORT
        kcm_access
)

ecm_qt_install_logging_categories(
    EXPORT kcm_access
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

target_link_libraries(kcm_access PRIVATE
    kaccess_settings
    Qt::DBus
    Qt::Quick
    Qt::GuiPrivate
    KF6::KCMUtils
    KF6::CoreAddons
    KF6::GlobalAccel
    KF6::GuiAddons
    KF6::I18n
    KF6::KCMUtilsQuick
    KF6::NotifyConfig
    KF6::XmlGui
)

install(
    FILES
        kcmaccessibilitybell.kcfg
        kcmaccessibilitykeyboard.kcfg
        kcmaccessibilitymouse.kcfg
        kcmaccessibilityshakecursor.kcfg
        kcmaccessibilitycolorblindnesscorrection.kcfg
        kcmaccessibilityinvert.kcfg
        kcmaccessibilityzoommagnifier.kcfg
    DESTINATION
        ${KDE_INSTALL_KCFGDIR}
)
