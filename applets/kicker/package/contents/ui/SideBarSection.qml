/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.draganddrop as DnD
import org.kde.kirigami as Kirigami

DnD.DropArea {
    id: root

    width: Kirigami.Units.iconSizes.medium
    height: contentHeight

    anchors.horizontalCenter: parent.horizontalCenter

    property int contentHeight: model ? (model.count * Kirigami.Units.iconSizes.medium) + ((model.count - 1) * flow.spacing) : 0

    property alias model: repeater.model

    onDragMove: event => {
        if (flow.animating) {
            return;
        }

        const above = flow.childAt(event.x, event.y),
              source = kicker.dragSource;

        if (above && source && above !== source && source.parent === flow) {
            repeater.model.moveRow(source.itemIndex, above.itemIndex);
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

        spacing: (2 * Kirigami.Units.smallSpacing)

        Repeater {
            id: repeater

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
