/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3
import org.kde.kirigami as Kirigami

Item {
    id: statusIcon

    required property string icon
    required property string label
    required property string tip
    required property string hint

    signal triggered(int /*Qt::MouseButton*/ button)

    readonly property int iconSize: Kirigami.Units.iconSizes.roundedIconSize(Math.min(width, height))

    opacity: hint === "disable" ? 0.3 : 1

    function extractLabelString(l: string): string {
        if (l.length >= 2 && l.charCodeAt(0) < 127 && l.charCodeAt(1) < 127) {
            return l.substring(0, 2);
        } else {
            return l.substring(0, 1);
        }
    }

    Kirigami.Icon {
        id: imageIcon
        anchors.centerIn: parent
        width: statusIcon.iconSize
        height: statusIcon.iconSize
        scale: mouseArea.pressed ? 0.9 : 1
        source: statusIcon.icon
        visible: statusIcon.icon.length > 0
        animated: false
        // active: mouseArea.containsMouse
    }
    PlasmaComponents3.Label {
        id: textIcon
        anchors.centerIn: parent
        width: statusIcon.iconSize
        height: statusIcon.iconSize
        scale: mouseArea.pressed ? 0.9 : 1
        // a reasonable large size to make Text.Fit work
        minimumPointSize: 0
        font.pointSize: 1024
        fontSizeMode: Text.Fit
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        text: statusIcon.extractLabelString(statusIcon.label)
        textFormat: Text.PlainText
        visible: statusIcon.icon.length === 0
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: mouse => {
            statusIcon.triggered(mouse.button);
        }

        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            mainText: statusIcon.label
            subText: statusIcon.tip
            icon: statusIcon.icon
        }
    }
}
