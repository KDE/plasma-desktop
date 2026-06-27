# SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>
# SPDX-License-Identifier: LGPL-2.0-or-later

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_workspacepresets\")

kcmutils_add_qml_kcm(kcm_workspacepresets)

target_sources(kcm_workspacepresets PRIVATE
    workspacepresets.cpp
    presetsmodel.cpp
)

qt_add_resources(WORKSPACEPRESETS_RESOURCES resources.qrc)
target_sources(kcm_workspacepresets PRIVATE ${WORKSPACEPRESETS_RESOURCES})

target_link_libraries(kcm_workspacepresets PRIVATE
    Qt6::DBus
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtils
    KF6::KCMUtilsQuick
)
