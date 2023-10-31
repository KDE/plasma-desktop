/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Shapes

import org.kde.kirigami as Kirigami

Rectangle {
    id: root

    implicitWidth: 220
    implicitHeight: 220

    required property var device
    readonly property color lightColor: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.2)

    readonly property int markWidth: 5

    readonly property real axisXValue: device !== null ? device.axisValue.x : 0
    readonly property real axisYValue: device !== null ? device.axisValue.y : 0

    readonly property real axisX: (axisXValue * root.width / 2) + (root.width / 2)
    readonly property real axisY: (axisYValue * root.height / 2) + (root.height / 2)

    border {
        color: Kirigami.Theme.textColor
        width: 1
    }

    color: "transparent"

    Shape {
        anchors.fill: parent

        // horizontal middle separator
        ShapePath {
            strokeWidth: 1
            strokeColor: root.lightColor

            startX: 0
            startY: root.height / 2

            PathLine {
                x: root.width
                y: root.height / 2
            }
        }

        // vertical middle separator
        ShapePath {
            strokeWidth: 1
            strokeColor: root.lightColor

            startX: root.width / 2
            startY: 0

            PathLine {
                x: root.width / 2
                y: root.height
            }
        }

        // first part of the marker
        ShapePath {
            strokeWidth: 1
            strokeColor: Kirigami.Theme.textColor

            startX: root.axisX - root.markWidth
            startY: root.axisY - root.markWidth

            PathLine {
                x: root.axisX + root.markWidth
                y: root.axisY + root.markWidth
            }
        }

        // second part of the marker
        ShapePath {
            strokeWidth: 1
            strokeColor: Kirigami.Theme.textColor

            startX: root.axisX + root.markWidth
            startY: root.axisY - root.markWidth

            PathLine {
                x: root.axisX - root.markWidth
                y: root.axisY + root.markWidth
            }
        }
    }
}