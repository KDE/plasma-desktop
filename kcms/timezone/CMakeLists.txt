# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: BSD-2-Clause

add_definitions(-DTRANSLATION_DOMAIN="kcm_timezone")

kcmutils_add_qml_kcm(kcm_timezone INSTALL_NAMESPACE "plasma/kcms/systemsettings")

target_sources(kcm_timezone PRIVATE
    kcm.cpp
    timezonehandler.cpp
)

target_link_libraries(kcm_timezone PRIVATE
    KF6::CoreAddons
    KF6::KCMUtils
    KF6::KCMUtilsQuick
    KF6::I18n
    Qt6::Quick
    Qt6::QuickWidgets
)
