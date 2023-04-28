add_definitions(-DTRANSLATION_DOMAIN=\"kcmqtquicksettings\")

kcoreaddons_add_plugin(kcm_qtquicksettings INSTALL_NAMESPACE "plasma/kcms/systemsettings_qwidgets")
kcmutils_generate_desktop_file(kcm_qtquicksettings)

target_sources(kcm_qtquicksettings PRIVATE kcmqtquicksettings.cpp kcmqtquicksettings.h)
kconfig_add_kcfg_files(kcm_qtquicksettings renderersettings.kcfgc)
ki18n_wrap_ui(kcm_qtquicksettings kcmqtquicksettingswidget.ui)

target_link_libraries(kcm_qtquicksettings
    Qt::Widgets
    KF6::WidgetsAddons
    KF6::KCMUtils
    KF6::I18n
)
