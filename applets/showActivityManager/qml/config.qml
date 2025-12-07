/*
    SPDX-FileCopyrightText: 2020 Ivan Čukić <ivan.cukic at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
         name: i18nc("@title page", "Appearance") // qmllint disable unqualified
         icon: "activities"
         source: "ConfigAppearance.qml"
    }
}
