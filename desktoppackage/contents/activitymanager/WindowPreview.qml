/*   vim:set foldmethod=marker:
 *
 *   Copyright (C) 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

        onClicked: console.log("Clicked")
    }
}


