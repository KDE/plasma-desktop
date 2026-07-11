/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2012 Dan Vratil <dvratil@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later

    Adapted from KScreen
*/

import QtQuick
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import org.kde.kirigami as Kirigami

// Visually represents a monitor
Item {
    id: output

    default property alias children: orientationPanel.children

    // The resolution of the screen
    required property size screenSize
    // Aspect ratio of the screen
    readonly property real aspectRatio: screenSize.width / screenSize.height

    // The width of the representative screen
    readonly property real outputWidth: orientationPanel.width
    // Calculate the needed height of the representative screen to accurately reflect the screen's aspect ratio
    readonly property real outputHeight: (screenSize.height / screenSize.width) * outputWidth

    // Padding between the items
    readonly property real padding: Kirigami.Units.smallSpacing

    implicitHeight: outputHeight + Kirigami.Units.gridUnit + padding * 3

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
            id: orientationPanel
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                margins: Kirigami.Units.smallSpacing
            }

            height: output.outputHeight
            color: Kirigami.Theme.backgroundColor

            border {
                color: Kirigami.Theme.disabledTextColor
                width: 1
            }
        }
    }
}

