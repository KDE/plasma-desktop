/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.plasmoid
import org.kde.plasma.configuration

ConfigModel {
    id: configModel

    readonly property string layoutConfigSource: ""

    ConfigCategory {
         name: i18nc("@title:group for configuration dialog page", "Location")
         icon: "inode-directory"
         source: "ConfigLocation.qml"
    }

    ConfigCategory {
         name: i18nc("@title:group for configuration dialog page", "Icons")
         icon: "preferences-desktop-icons"
         source: "ConfigIcons.qml"
    }

    ConfigCategory {
         name: i18nc("@title:group for configuration dialog page", "Filter")
         icon: "preferences-desktop-filter"
         source: "ConfigFilter.qml"
    }
}
