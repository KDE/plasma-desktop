/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: statusIcon
    property string icon;
    property string label;
    property string tip;
    property string hint;
    signal triggered(variant button);
    property int iconSize: PlasmaCore.Units.roundToIconSize(Math.min(width, height))

    opacity: 'disable' == hint ? 0.3 : 1

    function extractLabelString(l) {
        if (l.length >= 2 && l.charCodeAt(0) < 127 && l.charCodeAt(1) < 127) {
            return l.substring(0, 2);
        } else {
             return l.substring(0, 1);
        }
    }

    PlasmaCore.IconItem {
        id: imageIcon
        anchors.centerIn: parent
        width: iconSize
        height: iconSize
        scale: mouseArea.pressed ? 0.9 : 1
        source: statusIcon.icon
        visible: statusIcon.icon.length > 0
        animated: false
        // active: mouseArea.containsMouse
    }
    PlasmaComponents.Label {
        id: textIcon
        anchors.centerIn: parent
        width: iconSize
        height: iconSize
        scale: (mouseArea.pressed ? 0.9 : 1)
        // a reasonable large size to make Text.Fit work
        minimumPointSize: 0
        font.pointSize: 1024
        fontSizeMode: Text.Fit
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        text: extractLabelString(label)
        visible: icon.length == 0
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
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
