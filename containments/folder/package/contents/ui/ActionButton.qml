/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaCore.ToolTipArea {
    id: button

    location: PlasmaCore.Types.LeftEdge
    mainText: action !== undefined ? action.text : ""
    mainItem: toolTipDelegate

    //API
    property QtObject svg
    property alias elementId: icon.elementId
    property QtObject action
    property bool backgroundVisible: false
    property int iconSize: 32
    property int pressedOffset: 1
    property bool checked: false
    property bool toggle: false
    property string text
    signal clicked

    width: buttonRow.width
    height: buttonRow.height

    opacity: action==undefined||action.enabled?1:0.6

    Behavior on opacity {
        NumberAnimation {
            duration: units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    onCheckedChanged: {
        if (checked) {
            buttonItem.elementId = "pressed"
            shadowItem.opacity = 0
        } else {
            buttonItem.elementId = "normal"
            shadowItem.opacity = 1
        }
    }

    PlasmaCore.Svg {
        id: buttonSvg
        imagePath: "widgets/actionbutton"
    }

    PlasmaCore.SvgItem {
        id: shadowItem
        svg: buttonSvg
        elementId: "shadow"
        width: iconSize+13//button.backgroundVisible?iconSize+8:iconSize
        height: width
        visible: button.backgroundVisible
    }

    Row {
        id: buttonRow

        Item {
            width: buttonItem.visible?buttonItem.width:iconSize
            height: buttonItem.visible?buttonItem.height:iconSize

            PlasmaCore.SvgItem {
                id: buttonItem
                svg: buttonSvg
                elementId: "normal"
                width: shadowItem.width
                height: shadowItem.height
                visible: backgroundVisible
            }

            PlasmaCore.SvgItem {
                id: icon
                width: iconSize
                height: iconSize
                svg: button.svg
                anchors.centerIn: parent
            }
        }

        Text {
            id: actionText
            text: button.text
            style: Text.Outline
            color: theme.textColor
            styleColor: Qt.rgba(1,1,1,0.4)
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    MouseArea {
        anchors.fill: parent
        anchors.leftMargin: -10
        anchors.topMargin: -10
        anchors.rightMargin: -10
        anchors.bottomMargin: -10
        preventStealing: true
        onPressed: {
            buttonItem.elementId = "pressed"
            shadowItem.opacity = 0;
            button.x = button.x + button.pressedOffset;
            button.y = button.y + button.pressedOffset;
        }
        onReleased: {
            if (button.checked || !button.toggle) {
                buttonItem.elementId = "normal"
                shadowItem.opacity = 1
                button.checked = false
            } else {
                button.checked = true
            }
            button.x = button.x - button.pressedOffset;
            button.y = button.y - button.pressedOffset;
        }
        onClicked: {
            if (action) {
                action.trigger()
            } else {
                button.clicked()
            }
        }
    }
    Rectangle { color: "white"; opacity: 0.4; visible: debug; anchors.fill: parent; }
}
