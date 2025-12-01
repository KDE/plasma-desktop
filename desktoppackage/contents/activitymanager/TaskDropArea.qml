/*
    SPDX-FileCopyrightText: 2020 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.draganddrop as DND
import org.kde.plasma.extras as PlasmaExtras

DND.DropArea {
    id: root

    signal taskDropped(var mimeData, var modifiers)
    signal clicked()
    signal entered()

    property int topPadding: 0
    property string activityName: ""
    property bool selected: false
    property string actionTitle: ""
    property bool isHovered: false
    property bool actionVisible: false

    PlasmaExtras.Highlight {
        id: dropHighlight
        anchors {
            fill: parent
            // topMargin: icon.height + 3 * Kirigami.Units.smallSpacing
            topMargin: root.topPadding
        }
        visible: root.isHovered
        z: -1
    }

    Text {
        id: dropAreaLeftText
        anchors {
            fill: dropHighlight
            leftMargin: Kirigami.Units.gridUnit
            rightMargin: Kirigami.Units.gridUnit
        }

        color: Kirigami.Theme.textColor
        visible: root.actionVisible

        text: root.actionTitle
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        maximumLineCount: 3
    }

    anchors {
        left: parent.left
        right: parent.horizontalCenter
        top: parent.top
        bottom: parent.bottom
    }

    preventStealing: true
    enabled: true

    onDrop: {
        root.taskDropped(event.mimeData, event.modifiers);
    }

    onDragEnter: {
        root.isHovered = true;
    }

    onDragLeave: {
        root.isHovered = false;
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
        hoverEnabled: true
        onEntered: root.entered()

        Accessible.name: root.activityName
        Accessible.role: Accessible.Button
        Accessible.selected: root.selected
        Accessible.onPressAction: root.clicked()
    }
}
