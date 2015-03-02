/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.2
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.SvgItem {
    id: actionButton

    width: visible ? units.iconSizes.small : 0
    height: width

    signal clicked

    property string element

    svg: actionOverlays
    elementId: element + "-normal"

    Behavior on opacity {
        NumberAnimation { duration: units.shortDuration }
    }

    MouseArea {
        id: actionButtonMouseArea

        anchors.fill: actionButton

        acceptedButtons: Qt.LeftButton
        hoverEnabled: true

        onClicked: actionButton.clicked()

        states: [
            State {
                name: "hover"
                when: actionButtonMouseArea.containsMouse && !pressed

                PropertyChanges {
                    target: actionButton
                    elementId: actionButton.element + "-hover"
                }
            },
            State {
                name: "pressed"
                when: actionButtonMouseArea.pressed

                PropertyChanges {
                    target: actionButton
                    elementId: actionButton.element + "open-pressed"
                }
            }
        ]
    }
}
