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
import QtQuick.Controls 1.4 as Controls

import org.kde.plasma.core 2.0 as PlasmaCore

Column {
    spacing: units.smallSpacing / 2
    property alias label: textlabel.text
    property alias model: repeater.model
    property alias current: exlGroupbox.current

    function itemAt(index) {
        return repeater.itemAt(index)
    }

    Controls.Label {
        id: textlabel
    }

    Controls.ExclusiveGroup { id: exlGroupbox }
    Column {
        leftPadding: units.smallSpacing
        spacing: units.smallSpacing / 2

        Repeater {
            id: repeater
            Controls.RadioButton {
                text: modelData
                exclusiveGroup: exlGroupbox

                property alias tooltiptext: tooltip.text

                ToolTip {
                    id: tooltip
                }
            }
        }
    }
}
