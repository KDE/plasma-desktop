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

kcoreaddons_add_plugin(kcm_access SOURCES ${kcm_access_PART_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file(kcm_access)

target_link_libraries(kcm_access
    Qt::DBus
    Qt::Quick
    KF6::KCMUtils
    KF6::CoreAddons
    KF6::Declarative
    KF6::GuiAddons
    KF6::I18n
    KF6::NewStuffCore
    KF6::KCMUtilsQuick
    KF6::WindowSystem
    KF6::NotifyConfig
    X11::X11
)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(kcm_access Qt::X11Extras)
else()
    target_link_libraries(kcm_access Qt::GuiPrivate)
endif()

install(
    FILES
        kcmaccessibilitybell.kcfg
        kcmaccessibilitykeyboard.kcfg
        kcmaccessibilitymouse.kcfg
        kcmaccessibilityscreenreader.kcfg
    DESTINATION
        ${KDE_INSTALL_KCFGDIR}
)

kpackage_install_package(package kcm_access kcms)
