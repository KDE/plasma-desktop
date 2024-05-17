/*
    SPDX-FileCopyrightText: 2020 Tranter Madi <trmdi@yandex.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick

MouseArea {
    id: root

    required property Text textItem

    onTextItemChanged: {
        textItem.parent = this;
        textItem.width = Qt.binding(() => width);
    }

    clip: textItem.elide === Text.ElideNone
    hoverEnabled: true

    onContainsMouseChanged: {
        if (!containsMouse) {
            state = "";
        }
    }

    Timer {
        id: timer
        interval: 500
        running: root.containsMouse
        onTriggered: {
            if (root.width < root.textItem.implicitWidth) {
                root.state = "ShowRight";
            }
        }
    }

    states: [
        State {
            name: ""
            PropertyChanges {
                target: root.textItem
                x: 0
            }
        },
        State {
            name: "ShowRight"
            PropertyChanges {
                target: root.textItem
                x: root.width - root.textItem.implicitWidth
            }
        }
    ]

    transitions: Transition {
        to: "ShowRight"
        NumberAnimation {
            target: root.textItem
            properties: "x"
            easing.type: Easing.Linear
            duration: Math.abs(root.textItem.implicitWidth - root.width) * 25
        }
    }
}
