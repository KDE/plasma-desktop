add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.taskmanager\")

plasma_install_package(package org.kde.plasma.taskmanager)

ecm_add_qml_module(taskmanagerplugin
    URI org.kde.plasma.private.taskmanager
    GENERATE_PLUGIN_SOURCE
)

kconfig_add_kcfg_files(taskmanagerplugin ${CMAKE_SOURCE_DIR}/kcms/recentFiles/kactivitymanagerd_plugins_settings.kcfgc)

ecm_qt_declare_logging_category(taskmanagerplugin
    HEADER log_settings.h
    IDENTIFIER TASKMANAGER_DEBUG
    CATEGORY_NAME org.kde.plasma.taskmanager)

target_sources(taskmanagerplugin PRIVATE
    backend.cpp
    smartlauncherbackend.cpp
    smartlauncheritem.cpp
)

# FIXME Cleanup no longer used libs.
target_link_libraries(taskmanagerplugin PRIVATE
                      Qt::Core
                      Qt::Qml
                      Qt::Quick
                      Plasma::Activities
                      Plasma::ActivitiesStats
                      KF6::ConfigGui
                      KF6::I18n
                      KF6::KIOCore
                      KF6::KIOGui
                      KF6::KIOFileWidgets # KFilePlacesModel
                      KF6::Notifications # KNotificationJobUiDelegate
                      Plasma::Plasma
                      KSysGuard::ProcessCore
                      KF6::Service
                      KF6::WindowSystem
                      PW::LibNotificationManager)

ecm_finalize_qml_module(taskmanagerplugin)
