# KRunner Settings Module

set(kcm_krunnersettings_SRCS
    krunnersettings.cpp
)
kcmutils_generate_module_data(
    kcm_krunnersettings_SRCS
    MODULE_DATA_HEADER krunnersettingsdata.h
    MODULE_DATA_CLASS_NAME KRunnerSettingsData
    SETTINGS_HEADERS krunnersettingsbase.h
    SETTINGS_CLASSES KRunnerSettingsBase
)

kconfig_add_kcfg_files(kcm_krunnersettings_SRCS krunnersettingsbase.kcfgc GENERATE_MOC)

kcoreaddons_add_plugin(kcm_krunnersettings SOURCES ${kcm_krunnersettings_SRCS} INSTALL_NAMESPACE "plasma/kcms/desktop")
kcmutils_generate_desktop_file(kcm_krunnersettings)

target_link_libraries(kcm_krunnersettings
    Qt::DBus
    KF6::Activities
    KF6::ConfigCore
    KF6::I18n
    KF6::KCMUtils
    KF6::KCMUtilsQuick
)

install(FILES krunnersettingsbase.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
kpackage_install_package(package kcm_krunnersettings kcms)
