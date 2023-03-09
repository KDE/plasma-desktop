# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

add_library(kcm_plasmasearch MODULE kcm.cpp)
kcmutils_generate_desktop_file(kcm_plasmasearch)

target_link_libraries(kcm_plasmasearch
  KF6::CoreAddons
  KF6::KCMUtilsCore
  KF6::KCMUtils
  KF6::KCMUtilsQuick
  KF6::Runner
  KF6::I18n
  Qt::DBus
)

kpackage_install_package(package kcm_plasmasearch kcms)
install(TARGETS kcm_plasmasearch DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings)
