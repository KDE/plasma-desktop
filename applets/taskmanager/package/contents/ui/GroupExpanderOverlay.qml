/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.SvgItem {
    id: arrow

    anchors {
        bottom: arrow.parent.bottom
        horizontalCenter: iconBox.horizontalCenter
    }

    visible: parent.m.IsGroupParent === true

    states: [
        State {
            name: "top"
            when: plasmoid.location === PlasmaCore.Types.TopEdge
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
            when: plasmoid.location === PlasmaCore.Types.LeftEdge
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
            when: plasmoid.location === PlasmaCore.Types.RightEdge
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

    svg: taskSvg
    elementId: elementForLocation()

    function elementForLocation() {
        switch (plasmoid.location) {
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
