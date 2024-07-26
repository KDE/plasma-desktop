/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <joshua.gons@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import org.kde.kirigami as Kirigami

Item {
    id: output

    default property alias children: childrenContainer.children
    property alias internalRect: rect

    required property real outputWidth
    required property real outputHeight

    readonly property real padding: 2

    width: outputWidth + padding * 2
    height: outputHeight + Kirigami.Units.gridUnit + padding * 3

    Rectangle {
        id: outline

        anchors.fill: parent

        radius: Kirigami.Units.cornerRadius
        color: Qt.alpha(Kirigami.Theme.disabledTextColor, 0.1)

        border {
            color: Kirigami.Theme.disabledTextColor
            width: 1

            ColorAnimation on color {
                duration: Kirigami.Units.longDuration
            }
        }

        Rectangle {
            anchors {
                left: parent.left
                leftMargin: Kirigami.Units.largeSpacing
                top: parent.top
                topMargin: Kirigami.Units.smallSpacing
            }

            width: 35
            height: 5
            color: Kirigami.Theme.backgroundColor

            border {
                color: Kirigami.Theme.disabledTextColor
                width: 1
            }
        }

        Rectangle {
            id: rightButton

            anchors {
                right: parent.right
                rightMargin: Kirigami.Units.largeSpacing
                top: parent.top
                topMargin: Kirigami.Units.smallSpacing
            }

            width: 35
            height: 5
            color: "transparent"

            border {
                color: Kirigami.Theme.disabledTextColor
                width: 1
            }
        }

        Rectangle {
            id: rect
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                top: rightButton.bottom
                margins: Kirigami.Units.smallSpacing
            }

            height: outputHeight
            width: outputWidth
            color: Kirigami.Theme.activeBackgroundColor
            opacity: 0.8

            border {
                color: Kirigami.Theme.highlightColor
                width: 1
            }
        }

        Item {
            id: childrenContainer
        }
    }
}

