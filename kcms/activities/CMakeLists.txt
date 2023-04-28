add_definitions(-DTRANSLATION_DOMAIN=\"kcm_activities5\")

set(KAMD_KCM_DATADIR ${KDE_INSTALL_DATADIR}/kactivitymanagerd/workspace/settings/)
if(NOT IS_ABSOLUTE "${KDE_INSTALL_DATADIR}")
   set (KAMD_KCM_DATADIR "${CMAKE_INSTALL_PREFIX}/${KAMD_KCM_DATADIR}")
endif ()

configure_file(kactivities-kcm-features.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kactivities-kcm-features.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}) # utils folder

add_subdirectory(imports)

qt_add_dbus_interface(KAMD_KCM_DBUS_SRCS common/dbus/org.kde.ActivityManager.Features.xml features_interface)
qt_add_dbus_interface(KAMD_KCM_DBUS_SRCS common/dbus/org.kde.ActivityManager.ResourceScoring.xml resourcescoring_interface)

kcoreaddons_add_plugin(kcm_activities INSTALL_NAMESPACE "plasma/kcms/systemsettings_qwidgets")
kcmutils_generate_desktop_file(kcm_activities)
target_sources(kcm_activities PRIVATE
   kcm_activities.cpp
   MainConfigurationWidget.cpp
   ActivitiesTab.cpp
   SwitchingTab.cpp
   ExtraActivitiesInterface.cpp
)

ki18n_wrap_ui(kcm_activities
   ui/MainConfigurationWidgetBase.ui
   ui/SwitchingTabBase.ui
)
kcmutils_generate_module_data(kcm_activities
    MODULE_DATA_HEADER kactivitiesdata.h
    MODULE_DATA_CLASS_NAME KActivitiesData
    SETTINGS_HEADERS kactivitymanagerd_settings.h kactivitymanagerd_plugins_settings.h
    SETTINGS_CLASSES KActivityManagerdSettings KActivityManagerdPluginsSettings
)
kconfig_add_kcfg_files(kcm_activities kactivitymanagerd_settings.kcfgc GENERATE_MOC)
kconfig_add_kcfg_files(kcm_activities kactivitymanagerd_plugins_settings.kcfgc GENERATE_MOC)

target_link_libraries(kcm_activities
   Qt::Quick
   Qt::QuickWidgets
   Qt::Core
   Qt::Sql
   Qt::Qml
   Qt::DBus
   KF6::KCMUtils
   KF6::I18n
   KF6::Service
   KF6::XmlGui
   KF6::GlobalAccel
   KF6::Activities
)

install(DIRECTORY qml/activitiesTab DESTINATION ${KAMD_KCM_DATADIR}/qml)

install(FILES kactivitymanagerd_settings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kactivitymanagerd_plugins_settings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
