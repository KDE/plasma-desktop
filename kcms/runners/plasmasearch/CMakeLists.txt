# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

add_library(kcm_plasmasearch MODULE kcm.cpp)

target_link_libraries(kcm_plasmasearch
  KF5::CoreAddons
  KF5::KCMUtilsCore
  KF5::KCMUtils
  KF5::QuickAddons
  KF5::Runner
  KF5::I18n
  Qt::DBus
)

kpackage_install_package(package kcm_plasmasearch kcms)
install(TARGETS kcm_plasmasearch DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings)
install(FILES kcm_plasmasearch.desktop DESTINATION ${KDE_INSTALL_APPDIR})
