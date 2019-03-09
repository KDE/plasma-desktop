/***************************************************************************
 *   Copyright (C) 2012-2013 by Eike Hein <hein@kde.org>                   *
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

import QtQuick 2.0

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
