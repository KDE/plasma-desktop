/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.plasmoid
import org.kde.plasma.configuration

ConfigModel {
    property bool isFolder: (Plasmoid.pluginName === "org.kde.plasma.folder")

    ConfigCategory {
         name: i18n("Location")
         icon: "inode-directory"
         source: "ConfigLocation.qml"
         visible: isFolder
    }

    ConfigCategory {
         name: i18n("Icons")
         icon: "preferences-desktop-icons"
         source: "ConfigIcons.qml"
         visible: isFolder
    }

    ConfigCategory {
         name: i18n("Filter")
         icon: "preferences-desktop-filter"
         source: "ConfigFilter.qml"
         visible: isFolder
    }
}
