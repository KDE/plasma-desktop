/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.plasmoid
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg

KSvg.SvgItem {
    id: actionButton

    width: {
        if (!visible) {
            return 0;
        }
        switch (Plasmoid.configuration.iconSize) {
            case 0: return Kirigami.Units.iconSizes.small;
            case 1: return Kirigami.Units.iconSizes.small;
            case 2: return Kirigami.Units.iconSizes.smallMedium;
            case 3: return Kirigami.Units.iconSizes.smallMedium;
            case 4: return Kirigami.Units.iconSizes.smallMedium;
            case 5: return Kirigami.Units.iconSizes.medium;
            case 6: return Kirigami.Units.iconSizes.large;
            default: return Kirigami.Units.iconSizes.small;
        }
    }
    height: width

    signal clicked()

    property string element

    svg: KSvg.Svg {
        imagePath: "widgets/action-overlays"
        multipleImages: true
        size: "16x16"
    }
    elementId: element + "-normal"

    Behavior on opacity {
        NumberAnimation { duration: Kirigami.Units.shortDuration }
    }

    MouseArea {
        id: actionButtonMouseArea

        anchors.fill: actionButton

        acceptedButtons: Qt.LeftButton
        hoverEnabled: true

        onClicked: mouse => actionButton.clicked()

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
