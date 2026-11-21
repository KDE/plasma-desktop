# SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_recentFiles\")

kcmutils_add_qml_kcm(kcm_recentFiles
    SOURCES
        kcm_recentfiles.cpp
        kcm_recentfiles.h
)

kconfig_target_kcfg_file(kcm_recentFiles
    GENERATE_MOC
    FILE kactivitymanagerd_plugins_settings.kcfg
    HEADER kactivitymanagerd_plugins_settings.h
    CLASS_NAME KActivityManagerdPluginsSettings
    MUTATORS DEFAULT_VALUE_GETTERS GENERATE_PROPERTIES PARENT_IN_CONSTRUCTOR NOTIFIERS
)

kcmutils_generate_module_data(kcm_recentFiles
    MODULE_DATA_HEADER kactivitiesdata.h
    MODULE_DATA_CLASS_NAME KActivitiesData
    SETTINGS_HEADERS kactivitymanagerd_plugins_settings.h
    SETTINGS_CLASSES KActivityManagerdPluginsSettings
)

qt_add_dbus_interface(kcm_recentFiles_DBUS_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.ActivityManager.ResourceScoring.xml resourcescoring_interface)
target_sources(kcm_recentFiles PRIVATE ${kcm_recentFiles_DBUS_SRCS})

target_link_libraries (kcm_recentFiles
   Qt::Quick
   Qt::DBus
   Qt::Sql
   KF6::KCMUtils
   KF6::I18n
   KF6::Service
   Plasma::Activities
)
