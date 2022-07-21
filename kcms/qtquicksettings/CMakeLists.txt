add_definitions(-DTRANSLATION_DOMAIN=\"kcmqtquicksettings\")

set(kcm_qtquicksettings_PART_SRCS kcmqtquicksettings.cpp kcmqtquicksettings.h)
kconfig_add_kcfg_files(kcm_qtquicksettings_PART_SRCS renderersettings.kcfgc)

ki18n_wrap_ui(kcm_qtquicksettings_PART_SRCS kcmqtquicksettingswidget.ui)

add_library(kcm_qtquicksettings MODULE ${kcm_qtquicksettings_PART_SRCS})
kcmutils_generate_desktop_file(kcm_qtquicksettings)

target_link_libraries(kcm_qtquicksettings
    Qt::Widgets
    KF5::WidgetsAddons
    KF5::KCMUtils
    KF5::I18n
)

########### install files ###############

install(TARGETS kcm_qtquicksettings  DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings_qwidgets)
