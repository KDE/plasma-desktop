/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Flow {
    property bool animating: false

    readonly property bool tasksGrowInOppositeDirection: Plasmoid.configuration.reverseMode
    readonly property bool isHorizontalPanel: Plasmoid.formFactor === PlasmaCore.Types.Horizontal

    layoutDirection: (tasksGrowInOppositeDirection && isHorizontalPanel)
        ? (Qt.application.layoutDirection === Qt.LeftToRight)
            ? Qt.RightToLeft
            : Qt.LeftToRight
        : Qt.application.layoutDirection

    property int rows: Math.floor(height / children[0].height)
    property int columns: Math.floor(width / children[0].width)

    move: Transition {
        SequentialAnimation {
            PropertyAction { target: taskList; property: "animating"; value: true }

            NumberAnimation {
                properties: "x, y"
                easing.type: Easing.OutQuad
                duration: PlasmaCore.Units.longDuration
            }

            PropertyAction { target: taskList; property: "animating"; value: false }
        }
    }
}
