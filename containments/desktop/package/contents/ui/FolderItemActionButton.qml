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

    width: {
        if (!visible) {
            return 0;
        }
        switch(plasmoid.configuration.iconSize) {
            case 0: return PlasmaCore.Units.iconSizes.small;
            case 1: return PlasmaCore.Units.iconSizes.small;
            case 2: return PlasmaCore.Units.iconSizes.smallMedium;
            case 3: return PlasmaCore.Units.iconSizes.smallMedium;
            case 4: return PlasmaCore.Units.iconSizes.smallMedium;
            case 5: return PlasmaCore.Units.iconSizes.medium;
            default: return PlasmaCore.Units.iconSizes.small;
        }
    }
    height: width

    signal clicked

    property string element

    svg: actionOverlays
    elementId: element + "-normal"

    Behavior on opacity {
        NumberAnimation { duration: PlasmaCore.Units.shortDuration }
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
                when: actionButtonMouseArea.containsMouse && !actionButtonMouseArea.pressed

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
                    elementId: actionButton.element + "-pressed"
                }
            }
        ]
    }
}
