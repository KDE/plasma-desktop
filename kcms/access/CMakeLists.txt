# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmaccess\")

kcmutils_add_qml_kcm(kcm_access SOURCES kcmaccess.cpp)

kconfig_add_kcfg_files(kcm_access
    kcmaccessibilitybell.kcfgc
    kcmaccessibilitykeyboard.kcfgc
    kcmaccessibilitykeyboardfilters.kcfgc
    kcmaccessibilitymouse.kcfgc
    kcmaccessibilityscreenreader.kcfgc
    GENERATE_MOC)

kcmutils_generate_module_data(kcm_access
    MODULE_DATA_HEADER kcmaccessibilitydata.h
    MODULE_DATA_CLASS_NAME AccessibilityData
    SETTINGS_HEADERS kcmaccessibilitybell.h kcmaccessibilitykeyboard.h kcmaccessibilitykeyboardfilters.h kcmaccessibilitymouse.h kcmaccessibilityscreenreader.h
    SETTINGS_CLASSES BellSettings KeyboardSettings KeyboardFiltersSettings MouseSettings ScreenReaderSettings
)

target_link_libraries(kcm_access
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
    DESTINATION
        ${KDE_INSTALL_KCFGDIR}
)
