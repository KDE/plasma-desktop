/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
         name: i18nc("@title:group for configuration dialog page", "Appearance")
         icon: "preferences-desktop-color"
         source: "ConfigAppearance.qml"
    }
}
