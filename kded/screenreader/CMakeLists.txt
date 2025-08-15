# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

kcoreaddons_add_plugin(screenreader INSTALL_NAMESPACE "kf6/kded")

target_sources(screenreader PRIVATE screenreader.cpp)

target_link_libraries(screenreader PRIVATE KF6::CoreAddons KF6::DBusAddons KF6::ConfigCore kaccess_settings)

install(FILES org.kde.screenreader.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kglobalaccel)
