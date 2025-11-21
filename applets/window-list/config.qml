/*
 *    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0

import org.kde.plasma.configuration 2.0

ConfigModel {
    ConfigCategory {
        name: i18n("Appearance")
        icon: "preferences-desktop-color"
        source: "ConfigAppearance.qml"
    }
    ConfigCategory {
        name: i18n("Behavior")
        icon: "preferences-desktop"
        source: "ConfigBehavior.qml"
    }
}
