/*
 *  Copyright 2016 John Salatas <jsalatas@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import QtQuick.Controls 1.0 as QtControls
import org.kde.kquickcontrolsaddons 2.0

Rectangle {
    id: button
    SystemPalette {
        id:palette
    }

    color: palette.button

    border {
        width: 1
        color: Qt.rgba(palette.text.r, palette.text.g, palette.text.b, 0.5)
    }
    radius: 5

    visible: name != "empty"

    property alias icon: iconElement.source
    property alias text: textElement.text
    property string name

    width: units.gridUnit * 6
    height: units.gridUnit * 5


    Item {
         anchors {
             margins: units.smallSpacing
             left: parent.left
             right: parent.right
             verticalCenter: parent.verticalCenter
         }

        height: childrenRect.height

        PlasmaCore.IconItem {
            id: iconElement

            anchors.horizontalCenter: parent.horizontalCenter
            width: units.iconSizes.medium
            height: width

            source: icon
        }

        QtControls.Label {
             id: textElement
             anchors {
                 top: iconElement.bottom
                 horizontalCenter: parent.horizontalCenter
             }

        }
    }
}
