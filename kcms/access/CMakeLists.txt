# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmaccess\")

set(kcm_access_PART_SRCS kcmaccess.cpp )

kconfig_add_kcfg_files(kcm_access_PART_SRCS
    kcmaccessibilitybell.kcfgc
    kcmaccessibilitykeyboard.kcfgc
    kcmaccessibilitykeyboardfilters.kcfgc
    kcmaccessibilitymouse.kcfgc
    kcmaccessibilityscreenreader.kcfgc
    GENERATE_MOC)

kcmutils_generate_module_data(
    kcm_access_PART_SRCS
    MODULE_DATA_HEADER kcmaccessibilitydata.h
    MODULE_DATA_CLASS_NAME AccessibilityData
    SETTINGS_HEADERS kcmaccessibilitybell.h kcmaccessibilitykeyboard.h kcmaccessibilitykeyboardfilters.h kcmaccessibilitymouse.h kcmaccessibilityscreenreader.h
    SETTINGS_CLASSES BellSettings KeyboardSettings KeyboardFiltersSettings MouseSettings ScreenReaderSettings
)

kcoreaddons_add_plugin(kcm_access SOURCES ${kcm_access_PART_SRCS} INSTALL_NAMESPACE "kcms")

target_link_libraries(kcm_access
    Qt::DBus
    KF5::KCMUtils
    KF5::CoreAddons
    KF5::Declarative
    KF5::GuiAddons
    KF5::I18n
    KF5::NewStuff
    KF5::QuickAddons
    KF5::WindowSystem
    KF5::NotifyConfig
    Qt::X11Extras
    X11::X11
)

kcoreaddons_desktop_to_json(kcm_access "kcm_access.desktop")

install(FILES kcm_access.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(
    FILES
        kcmaccessibilitybell.kcfg
        kcmaccessibilitykeyboard.kcfg
        kcmaccessibilitymouse.kcfg
        kcmaccessibilityscreenreader.kcfg
    DESTINATION
        ${KDE_INSTALL_KCFGDIR}
)

kpackage_install_package(package kcmaccess kcms)
