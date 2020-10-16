/***************************************************************************
 *   Copyright (C) 2016 by Eike Hein <hein@kde.org>                        *
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

import QtQuick 2.4

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: tab

    width: label.contentWidth + (PlasmaCore.Units.largeSpacing * 2)
    height: label.contentHeight + (PlasmaCore.Units.smallSpacing * 2)

    property int index: 0
    property bool active: false
    property alias text: label.text
    Accessible.name: text
    Accessible.role: Accessible.PageTab

    Rectangle {
        anchors.fill: parent

        color: tab.parent.focus ? theme.highlightColor : "black"

        opacity: tab.active ? 0.4 : 0.15
        Behavior on opacity { SmoothedAnimation { duration: PlasmaCore.Units.shortDuration; velocity: 0.01 } }
    }

    PlasmaExtras.Heading {
        id: label

        x: PlasmaCore.Units.largeSpacing

        elide: Text.ElideNone
        wrapMode: Text.NoWrap
        opacity: tab.active ? 1.0 : 0.6
        Behavior on opacity { SmoothedAnimation { duration: PlasmaCore.Units.shortDuration; velocity: 0.01 } }

        color: tab.parent.focus ? theme.highlightedTextColor : "white"

        level: 1
    }

    MouseArea {
        anchors.fill: parent

        hoverEnabled: true

        onClicked: {
            tab.parent.activeTab = tab.index;
        }

        onContainsMouseChanged: {
            tab.parent.containsMouseChanged(tab.index, containsMouse);
        }
    }
}
