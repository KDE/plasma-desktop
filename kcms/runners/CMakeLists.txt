# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_search\")

# Search Config Module
set(kcm_search_SRCS
  kcm.cpp
  krunnerdata.cpp
)

kconfig_add_kcfg_files(kcm_search_SRCS krunnersettingsbase.kcfgc)

add_library(kcm_plasmasearch MODULE ${kcm_search_SRCS})
add_executable(plugininstaller
        plugininstaller/main.cpp
        plugininstaller/AbstractJob.cpp
        plugininstaller/ScriptJob.cpp
        plugininstaller/ZypperRPMJob.cpp)
if(HAVE_PACKAGEKIT)
    target_sources(plugininstaller PUBLIC plugininstaller/PackageKitJob.cpp)
endif()

set_target_properties(plugininstaller PROPERTIES
        OUTPUT_NAME "krunner-plugininstaller"
        )


target_link_libraries(kcm_plasmasearch
  KF5::KIOWidgets
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::Runner
  KF5::I18n
  KF5::NewStuffWidgets

  Qt::DBus
  Qt::Widgets
)

target_link_libraries(plugininstaller
  KF5::CoreAddons
  KF5::I18n
  Qt::Widgets
  KF5::Service
  KF5::KIOCore
  KF5::KIOWidgets
)
if(HAVE_PACKAGEKIT)
    target_link_libraries(plugininstaller PK::packagekitqt5)
endif()

install(FILES kcm_plasmasearch.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(TARGETS kcm_plasmasearch DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings_qwidgets)
install(FILES krunner.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
install(TARGETS plugininstaller DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


# KRunner Settings Module
set(kcm_krunnersettings_SRCS
    krunnersettings.cpp
    krunnersettingsdata.cpp
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
target_link_libraries(kcm_krunnersettings
    KF5::Activities
    KF5::ConfigCore
    KF5::I18n
    KF5::KCMUtils
    KF5::QuickAddons

    Qt::DBus
)

install(FILES krunnersettingsbase.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
kpackage_install_package(package kcm_krunnersettings kcms)
