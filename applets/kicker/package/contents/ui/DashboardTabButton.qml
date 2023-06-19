/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid 2.0

Item {
    id: tab

    width: label.contentWidth + (Kirigami.Units.gridUnit * 2)
    height: label.contentHeight + (Kirigami.Units.smallSpacing * 2)

    property int index: 0
    property bool active: false
    property alias text: label.text
    Accessible.name: text
    Accessible.role: Accessible.PageTab

    Rectangle {
        anchors.fill: parent

        color: tab.parent.focus ? Kirigami.Theme.highlightColor : "black"

        opacity: tab.active ? 0.4 : 0.15
        Behavior on opacity { SmoothedAnimation { duration: Kirigami.Units.shortDuration; velocity: 0.01 } }
    }

    Kirigami.Heading {
        id: label

        x: Kirigami.Units.gridUnit

        elide: Text.ElideNone
        wrapMode: Text.NoWrap
        opacity: tab.active ? 1.0 : 0.6
        Behavior on opacity { SmoothedAnimation { duration: Kirigami.Units.shortDuration; velocity: 0.01 } }

        color: tab.parent.focus ? Kirigami.Theme.highlightedTextColor : "white"

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
