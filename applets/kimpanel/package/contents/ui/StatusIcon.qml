/*
 *  Copyright 2014 Weng Xuetian <wengxt@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
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
    property int iconSize: units.roundToIconSize(Math.min(parent.width, parent.height))

    opacity: 'disable' == hint ? 0.3 : 1

    function extractLabelString(l) {
        if (l.length >= 2 && l.charCodeAt(0) < 127 && l.charCodeAt(1) < 127) {
            return l.substring(0, 2);
        } else {
             return l.substring(0, 1);
        }
    }

    function iconPath(p) {
        if (p.length > 0) {
            if (p[0] === '/') {
                return p;
            } else {
                return "image://icon/" + p;
            }
        }
        return p;
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
