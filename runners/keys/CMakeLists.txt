# SPDX-FileCopyrightText: 2025 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_keys\")

kcoreaddons_add_plugin(krunner_keys SOURCES keysrunner.cpp INSTALL_NAMESPACE "kf6/krunner")
target_link_libraries(krunner_keys
    Qt::DBus
    KF6::I18n
    KF6::Runner
    kglobalaccelmodel
)
