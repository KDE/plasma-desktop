/*
*   Copyright 2020 by Tranter Madi <trmdi@yandex.com>
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
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.6

MouseArea {
    id: textWrapper

    default property Text textItem

    clip: textItem.elide === Text.ElideNone
    hoverEnabled: true

    onContainsMouseChanged: {
        if (!containsMouse) {
            state = ""
        }
    }

    Timer {
        id: timer
        interval: 500
        running: textWrapper.containsMouse
        onTriggered: {
            if (textWrapper.width < textItem.implicitWidth) {
                textWrapper.state = "ShowRight"
            }
        }
    }

    states: [
        State {
            name: ""
            PropertyChanges { target: textItem; x: 0 }
        },
        State {
            name: "ShowRight"
            PropertyChanges { target: textItem; x: textWrapper.width - textItem.implicitWidth }
        }
    ]

    transitions: Transition {
        to: "ShowRight"
        NumberAnimation {
            target: textItem; properties: "x";
            easing.type: Easing.Linear;
            duration: Math.abs(textItem.implicitWidth - textWrapper.width)*25
        }
    }
}
