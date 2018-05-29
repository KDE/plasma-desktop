/*
 * Copyright 2017 Roman Gilg <subdiff@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
