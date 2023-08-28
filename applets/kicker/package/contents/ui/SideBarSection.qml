/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami 2 as Kirigami

// To reflect children's activeFocus
FocusScope {
    id: section

    anchors.horizontalCenter: parent.horizontalCenter
    width: Kirigami.Units.iconSizes.medium
    height: contentHeight

    property real contentHeight: model ? (model.count * Kirigami.Units.iconSizes.medium) + ((model.count - 1) * flow.spacing) : 0
    readonly property alias repeater: repeater
    property alias model: repeater.model

    Timer {
        id: resetAnimationDurationTimer

        interval: 150
        repeat: false

        onTriggered: {
            flow.animationDuration = interval - 20;
        }
    }

    DropArea {
        anchors.fill: parent
        onPositionChanged: event => {
            if (flow.animating) {
                return;
            }

            const above = flow.childAt(event.x, event.y);

            if (above && above !== dragSource.sourceItem && dragSource.sourceItem.parent === flow) {
                repeater.model.moveRow(dragSource.sourceItem.itemIndex, above.itemIndex);
            }
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
}
