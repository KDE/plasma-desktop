add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.taskmanager\")

plasma_install_package(package org.kde.plasma.taskmanager)

set(taskmanagerplugin_SRCS
    plugin/backend.cpp
    plugin/taskmanagerplugin.cpp

    plugin/smartlaunchers/smartlauncherbackend.cpp
    plugin/smartlaunchers/smartlauncheritem.cpp
)

kconfig_add_kcfg_files(taskmanagerplugin_SRCS ${CMAKE_SOURCE_DIR}/kcms/recentFiles/kactivitymanagerd_plugins_settings.kcfgc)

ecm_qt_declare_logging_category(taskmanagerplugin_SRCS
    HEADER log_settings.h
    IDENTIFIER TASKMANAGER_DEBUG
    CATEGORY_NAME org.kde.plasma.taskmanager)

install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/taskmanager)

add_library(taskmanagerplugin SHARED ${taskmanagerplugin_SRCS})

# FIXME Cleanup no longer used libs.
target_link_libraries(taskmanagerplugin
                      Qt::Core
                      Qt::DBus
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

install(TARGETS taskmanagerplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/taskmanager)
