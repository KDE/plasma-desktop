/*
    SPDX-FileCopyrightText: 2020 Tranter Madi <trmdi@yandex.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15

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
