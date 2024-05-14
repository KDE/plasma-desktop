# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmaccess\")

kcmutils_add_qml_kcm(kcm_access SOURCES kcmaccess.cpp)

add_library(kaccess_settings STATIC)
set_property(TARGET kaccess_settings PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(kaccess_settings PUBLIC Qt::Core KF6::ConfigGui)

kconfig_add_kcfg_files(kaccess_settings
    kcmaccessibilitybell.kcfgc
    kcmaccessibilitykeyboard.kcfgc
    kcmaccessibilitykeyboardfilters.kcfgc
    kcmaccessibilitymouse.kcfgc
    kcmaccessibilityactivationgestures.kcfgc
    kcmaccessibilityscreenreader.kcfgc
    kcmaccessibilityshakecursor.kcfgc
    GENERATE_MOC)

kcmutils_generate_module_data(kcm_access
    MODULE_DATA_HEADER kcmaccessibilitydata.h
    MODULE_DATA_CLASS_NAME AccessibilityData
    SETTINGS_HEADERS kcmaccessibilitybell.h kcmaccessibilitykeyboard.h kcmaccessibilitykeyboardfilters.h kcmaccessibilitymouse.h kcmaccessibilityactivationgestures.h kcmaccessibilityscreenreader.h kcmaccessibilityshakecursor.h
    SETTINGS_CLASSES BellSettings KeyboardSettings KeyboardFiltersSettings MouseSettings ActivationGesturesSettings ScreenReaderSettings ShakeCursorSettings
)

target_link_libraries(kcm_access PRIVATE
    kaccess_settings
    Qt::DBus
    Qt::Quick
    Qt::GuiPrivate
    KF6::KCMUtils
    KF6::CoreAddons
    KF6::GuiAddons
    KF6::I18n
    KF6::NewStuffCore
    KF6::KCMUtilsQuick
    KF6::WindowSystem
    KF6::NotifyConfig
    X11::X11
)

install(
    FILES
        kcmaccessibilitybell.kcfg
        kcmaccessibilitykeyboard.kcfg
        kcmaccessibilitymouse.kcfg
        kcmaccessibilityscreenreader.kcfg
        kcmaccessibilityshakecursor.kcfg
    DESTINATION
        ${KDE_INSTALL_KCFGDIR}
)
