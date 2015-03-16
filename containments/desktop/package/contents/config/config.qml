/***************************************************************************
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.configuration 2.0

ConfigModel {
    property bool isFolder: (plasmoid.pluginName == "org.kde.plasma.folder")

    ConfigCategory {
         name: i18n("Location")
         icon: "folder"
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
         icon: "view-filter"
         source: "ConfigFilter.qml"
         visible: isFolder
    }

    ConfigCategory {
         name: i18n("Tweaks")
         icon: "preferences-other"
         source: "ConfigTweaks.qml"
         visible: plasmoid.configuration.showTweaks && ("containmentType" in plasmoid)
    }
}
