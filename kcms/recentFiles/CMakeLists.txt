# SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_recentFiles\")

kcmutils_add_qml_kcm(kcm_recentfiles
    SOURCES
        kcm_recentfiles.cpp
        kcm_recentfiles.h
        excludedapplicationsmodel.cpp
        excludedapplicationsmodel.h
)

kconfig_add_kcfg_files(kcm_recentfiles kactivitymanagerd_plugins_settings.kcfgc GENERATE_MOC)

ecm_qt_declare_logging_category(kcm_recentfiles
    HEADER kcms_recentfiles_logging.h
    IDENTIFIER KCM_RECENTFILES
    CATEGORY_NAME kcm_recentFiles
    DESCRIPTION "KCM for recent file usage tracking"
    EXPORT KCM_RECENTFILES
)

qt_add_dbus_interface(kcm_recentfiles_DBUS_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/org.kde.ActivityManager.ResourceScoring.xml resourcescoring_interface)
target_sources(kcm_recentfiles PRIVATE ${kcm_recentfiles_DBUS_SRCS})

target_link_libraries (kcm_recentfiles
   Qt::Quick
   Qt::DBus
   Qt::Sql
   KF6::KCMUtils
   KF6::I18n
   KF6::Service
   Plasma::Activities
)
