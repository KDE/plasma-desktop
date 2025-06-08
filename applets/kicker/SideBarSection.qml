/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.kirigami as Kirigami

DropArea {
    id: root

    implicitWidth: repeater.count ? repeater.itemAt(0).implicitWidth : (Kirigami.Units.iconSizes.medium + Kirigami.Units.smallSpacing * 2)
    implicitHeight: contentHeight

    property int contentHeight: model ? (model.count * implicitWidth) + ((model.count - 1) * flow.spacing) : 0

    property alias model: repeater.model

    readonly property SideBarItem bottomSideBarItem: repeater.itemAt(repeater.count - 1) as SideBarItem

    signal interactionConcluded
    signal itemFocused(Item item)

    onActiveFocusChanged: (repeater.itemAt(0) ?? KeyNavigation.down).forceActiveFocus(Qt.TabFocusReason)

    onPositionChanged: event => {
        if (flow.animating) {
            return;
        }

        const above = flow.childAt(event.x, event.y) as SideBarItem
        const eventSource = event.source as SideBarItem

        if (above && eventSource && above !== eventSource && eventSource.parent === flow) {
            repeater.model.moveRow(eventSource.itemIndex, above.itemIndex);
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

        spacing: 0

        Repeater {
            id: repeater

            delegate: SideBarItem {
                favoritesModel: repeater.model
                baseModel: repeater.model
                Keys.onUpPressed: event => {
                    if (index > 0) {
                        repeater.itemAt(index - 1).forceActiveFocus(Qt.TabFocusReason)
                    } else {
                        event.accepted = false
                    }
                }
                Keys.onDownPressed: event => {
                    if (index + 1 < repeater.count) {
                        repeater.itemAt(index + 1).forceActiveFocus(Qt.TabFocusReason)
                    } else {
                        event.accepted = false
                    }
                }
                onInteractionConcluded: root.interactionConcluded()
                onActiveFocusChanged: if (activeFocus) root.itemFocused(this)
            }

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
