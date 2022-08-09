/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.SvgItem {
    id: actionButton

    width: {
        if (!visible) {
            return 0;
        }
        switch (plasmoid.configuration.iconSize) {
            case 0: return PlasmaCore.Units.iconSizes.small;
            case 1: return PlasmaCore.Units.iconSizes.small;
            case 2: return PlasmaCore.Units.iconSizes.smallMedium;
            case 3: return PlasmaCore.Units.iconSizes.smallMedium;
            case 4: return PlasmaCore.Units.iconSizes.smallMedium;
            case 5: return PlasmaCore.Units.iconSizes.medium;
            case 6: return PlasmaCore.Units.iconSizes.large;
            default: return PlasmaCore.Units.iconSizes.small;
        }
    }
    height: width

    signal clicked()

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
