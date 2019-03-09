/***************************************************************************
 *   Copyright (C) 2013-2014 by Eike Hein <hein@kde.org>                   *
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
import org.kde.draganddrop 2.0

DropArea {
    id: root

    width: units.iconSizes.medium
    height: contentHeight

    anchors.horizontalCenter: parent.horizontalCenter

    property int contentHeight: model ? (model.count * units.iconSizes.medium) + ((model.count - 1) * flow.spacing) : 0

    property alias model: repeater.model
    property alias usesPlasmaTheme: repeater.usesPlasmaTheme

    onDragMove: {
        if (flow.animating) {
            return;
        }

        var above = flow.childAt(event.x, event.y);

        if (above && above !== kicker.dragSource && dragSource.parent == flow) {
            repeater.model.moveRow(dragSource.itemIndex, above.itemIndex);
        }

    }

    Flow {
        id: flow

        anchors.fill: parent

        property bool animating: false
        property int animationDuration: resetAnimationDurationTimer.interval

        move: Transition {
            SequentialAnimation {
                PropertyAction { target: flow; property: "animating"; value: true }

                NumberAnimation {
                    duration: flow.animationDuration
                    properties: "x, y"
                    easing.type: Easing.OutQuad
                }

                PropertyAction { target: flow; property: "animating"; value: false }
            }
        }

        spacing: (2 * units.smallSpacing)

        Repeater {
            id: repeater

            property bool usesPlasmaTheme: false

            delegate: SideBarItem {}

            onCountChanged: {
                flow.animationDuration = 0;
                resetAnimationDurationTimer.start();
            }
        }
    }

    Timer {
        id: resetAnimationDurationTimer

        interval: 150
        repeat: false

        onTriggered: {
            flow.animationDuration = interval - 20;
        }
    }
}
