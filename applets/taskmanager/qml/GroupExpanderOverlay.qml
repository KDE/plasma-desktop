/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg as KSvg
import org.kde.plasma.plasmoid

KSvg.SvgItem {
    id: arrow

    anchors {
        bottom: arrow.parent.bottom
        horizontalCenter: iconBox.horizontalCenter
    }

    visible: parent.model.IsGroupParent

    states: [
        State {
            name: "top"
            when: Plasmoid.location === PlasmaCore.Types.TopEdge
            AnchorChanges {
                target: arrow
                anchors.top: arrow.parent.top
                anchors.left: undefined
                anchors.right: undefined
                anchors.bottom: undefined
                anchors.horizontalCenter: iconBox.horizontalCenter
                anchors.verticalCenter: undefined
            }
        },
        State {
            name: "left"
            when: Plasmoid.location === PlasmaCore.Types.LeftEdge
            AnchorChanges {
                target: arrow
                anchors.top: undefined
                anchors.left: arrow.parent.left
                anchors.right: undefined
                anchors.bottom: undefined
                anchors.horizontalCenter: undefined
                anchors.verticalCenter: iconBox.verticalCenter
            }
        },
        State {
            name: "right"
            when: Plasmoid.location === PlasmaCore.Types.RightEdge
            AnchorChanges {
                target: arrow
                anchors.top: undefined
                anchors.left: undefined
                anchors.right: arrow.parent.right
                anchors.bottom: undefined
                anchors.horizontalCenter: undefined
                anchors.verticalCenter: iconBox.verticalCenter
            }
        }
    ]

    implicitWidth: Math.min(naturalSize.width, iconBox.width)
    implicitHeight: Math.min(naturalSize.height, iconBox.width)

    imagePath: "widgets/tasks"
    elementId: elementForLocation()

    function elementForLocation(): string {
        switch (Plasmoid.location) {
        case PlasmaCore.Types.LeftEdge:
            return "group-expander-left";
        case PlasmaCore.Types.TopEdge:
            return "group-expander-top";
        case PlasmaCore.Types.RightEdge:
            return "group-expander-right";
        case PlasmaCore.Types.BottomEdge:
        default:
            return "group-expander-bottom";
        }
    }
}
