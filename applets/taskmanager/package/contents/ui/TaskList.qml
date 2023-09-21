/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid 2.0

Grid {
    property bool animating: false

    layoutDirection: (Plasmoid.configuration.reverseMode && !tasks.vertical)
        ? (Qt.application.layoutDirection === Qt.LeftToRight)
            ? Qt.RightToLeft
            : Qt.LeftToRight
        : Qt.application.layoutDirection

    rows: tasks.vertical ? -1 : Math.floor(height / children[0].height)
    columns: tasks.vertical ? Math.floor(width / children[0].width) : -1

    move: Transition {
        SequentialAnimation {
            PropertyAction { target: taskList; property: "animating"; value: true }

            NumberAnimation {
                properties: "x, y"
                easing.type: Easing.OutQuad
                duration: Kirigami.Units.longDuration
            }

            PropertyAction { target: taskList; property: "animating"; value: false }
        }
    }
}
