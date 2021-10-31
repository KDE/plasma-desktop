/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0

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

        color: tab.parent.focus ? PlasmaCore.Theme.highlightColor : "black"

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

        color: tab.parent.focus ? PlasmaCore.Theme.highlightedTextColor : "white"

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
