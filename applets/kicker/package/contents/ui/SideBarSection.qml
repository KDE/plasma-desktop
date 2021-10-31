/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.draganddrop 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

DropArea {
    id: root

    width: PlasmaCore.Units.iconSizes.medium
    height: contentHeight

    anchors.horizontalCenter: parent.horizontalCenter

    property int contentHeight: model ? (model.count * PlasmaCore.Units.iconSizes.medium) + ((model.count - 1) * flow.spacing) : 0

    property alias model: repeater.model
    property alias usesPlasmaTheme: repeater.usesPlasmaTheme

    onDragMove: event => {
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

        spacing: (2 * PlasmaCore.Units.smallSpacing)

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
