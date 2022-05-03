/*   vim:set foldmethod=marker:

    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2

Image {
    id: main

    width: 480
    height: 425

    source: "images/window2.png"
    anchors.centerIn: parent

    Item {
        id: title


        width: titleText.width + 32
        height: titleText.height + 32

        Rectangle {
            anchors.fill: parent
            color: "black"
            radius: 8
            opacity: .7
        }

        Text {
            id: titleText
            color: "white"
            text: "Firefox"
            font.pointSize: 24

            anchors.centerIn: parent
        }



    }

    Drag.active: mouseArea.drag.active
    Drag.hotSpot.x: 32
    Drag.hotSpot.y: 32

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        drag {
            target: title
        }
    }
}


