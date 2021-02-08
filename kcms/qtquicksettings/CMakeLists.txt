add_definitions(-DTRANSLATION_DOMAIN=\"kcmqtquicksettings\")

set(kcm_qtquicksettings_PART_SRCS kcmqtquicksettings.cpp)
kconfig_add_kcfg_files(kcm_qtquicksettings_PART_SRCS renderersettings.kcfgc)

ki18n_wrap_ui(kcm_qtquicksettings_PART_SRCS kcmqtquicksettingswidget.ui)

add_library(kcm_qtquicksettings ${kcm_qtquicksettings_PART_SRCS})

target_link_libraries(kcm_qtquicksettings
    Qt::Widgets
    KF5::WidgetsAddons
    KF5::KCMUtils
    KF5::I18n
)

########### install files ###############

install(TARGETS kcm_qtquicksettings  DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES qtquicksettings.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR})
