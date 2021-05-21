/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7

import QtQuick.Controls.Private 1.0

MouseArea {
    anchors.fill: parent

    property string text: ""

    hoverEnabled: true
    acceptedButtons: Qt.NoButton

    onEntered: timer.start()
    onExited: timer.killTooltip()
    onPositionChanged: timer.resetTooltip()

    Timer {
        id: timer
        interval: 1000
        onTriggered: {
            Tooltip.showText(parent, Qt.point(mouseX, mouseY), text)
        }

        function killTooltip() {
            stop()
            Tooltip.hideText()
        }

        function resetTooltip() {
            restart()
            Tooltip.hideText()
        }
    }
}
