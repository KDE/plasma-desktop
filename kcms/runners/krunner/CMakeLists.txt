# KRunner Settings Module


kcmutils_add_qml_kcm(kcm_krunnersettings SOURCES krunnersettings.cpp INSTALL_NAMESPACE "plasma/kcms/desktop")

kcmutils_generate_module_data(kcm_krunnersettings
    MODULE_DATA_HEADER krunnersettingsdata.h
    MODULE_DATA_CLASS_NAME KRunnerSettingsData
    SETTINGS_HEADERS krunnersettingsbase.h
    SETTINGS_CLASSES KRunnerSettingsBase
)

kconfig_add_kcfg_files(kcm_krunnersettings krunnersettingsbase.kcfgc GENERATE_MOC)

target_link_libraries(kcm_krunnersettings PRIVATE
    Qt::DBus
    KF6::ConfigCore
    KF6::I18n
    KF6::KCMUtils
    KF6::KCMUtilsQuick
)

install(FILES krunnersettingsbase.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
