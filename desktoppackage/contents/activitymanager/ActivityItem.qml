/*
    SPDX-FileCopyrightText: 2014-2020 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddonsComponents

import org.kde.plasma.activityswitcher 1.0 as ActivitySwitcher

import org.kde.activities 0.1 as Activities
import org.kde.activities.settings 0.1

import "static.js" as S

Item {
    id: root

    property int innerPadding  : PlasmaCore.Units.largeSpacing

    property bool current      : false
    property bool selected     : false
    property bool stoppable    : true

    property alias title       : title.text
    property alias icon        : icon.source
    property alias hasWindows  : hasWindowsIndicator.visible

    z : current  ? 10 :
        selected ?  5 : 0

    property string activityId : ""

    property string background : ""

    onBackgroundChanged: if (background[0] !== '#') {
        // We have a proper wallpaper, hurroo!
        backgroundColor.visible = false;

    } else {
        // We have only a color
        backgroundColor.color = background;
        backgroundColor.visible = true;
    }

    signal clicked

    width  : 200
    // height : width * 1 / PlasmaCore.Units.displayAspectRatio
    // Marco removed displayAspectRatio
    height : width * 9.0 / 16.0

    Item {
        anchors {
            fill: parent
        }

        // Background until we get something real
        Rectangle {
            id: backgroundColor

            anchors.fill: parent
            // This is intentional - while waiting for the wallpaper,
            // we are showing a semi-transparent black background
            color: "black"

            opacity: root.selected ? .8 : .5
        }

        Image {
            id: backgroundWallpaper

            anchors.fill: parent

            visible: !backgroundColor.visible
            source: "image://wallpaperthumbnail/" + background
            sourceSize: Qt.size(width, height)
        }

        // Title and the icon

        Rectangle {
            id: shade

            width: parent.height
            height: parent.width

            anchors.centerIn: parent
            rotation: 90

            gradient: Gradient {
                GradientStop { position: 1.0; color: "black" }
                GradientStop { position: 0.0; color: "transparent" }
            }

            opacity : root.selected ? 0.5 : 1.0
        }

        Rectangle {
            id: currentActivityHighlight

            visible:  root.current

            border.width: root.current ? PlasmaCore.Units.smallSpacing : 0
            border.color: PlasmaCore.Theme.highlightColor

            z: 10

            anchors {
                fill: parent
                // Hide the rounding error on the bottom of the rectangle
                bottomMargin: -1
            }

            color: "transparent"
        }

        Item {
            id: titleBar

            anchors {
                top   : parent.top
                left  : parent.left
                right : parent.right

                leftMargin : 2 * PlasmaCore.Units.smallSpacing + 2
                topMargin  : 2 * PlasmaCore.Units.smallSpacing
            }

            Text {
                id: title

                color   : "white"
                elide   : Text.ElideRight
                visible : shade.visible

                font.bold : true

                anchors {
                    top   : parent.top
                    left  : parent.left
                    right : icon.left
                }
            }

            Text {
                id: description

                color   : "white"
                elide   : Text.ElideRight
                text    : model.description
                opacity : .6

                anchors {
                    top   : title.bottom
                    left  : parent.left
                    right : icon.left
                }
            }

            PlasmaCore.IconItem {
                id: icon

                width   : PlasmaCore.Units.iconSizes.medium
                height  : width

                anchors {
                    right       : parent.right
                    rightMargin : 2 * PlasmaCore.Units.smallSpacing
                }
            }
        }

        Column {
            id: statsBar

            height: childrenRect.height + PlasmaCore.Units.smallSpacing

            anchors {
                bottom : controlBar.top
                left   : parent.left
                right  : parent.right

                leftMargin   : 2 * PlasmaCore.Units.smallSpacing + 2
                rightMargin  : 2 * PlasmaCore.Units.smallSpacing
                bottomMargin : PlasmaCore.Units.smallSpacing
            }

            PlasmaCore.IconItem {
                id      : hasWindowsIndicator
                source  : "window-duplicate"
                width   : 16
                height  : width
                opacity : .6
                visible : false
            }

            Text {
                id: lastUsedDate

                color   : "white"
                elide   : Text.ElideRight
                opacity : .6

                text: root.current ?
                        i18nd("plasma_shell_org.kde.plasma.desktop", "Currently being used") :
                        model.lastTimeUsedString
            }
        }

        Rectangle {
            id: dropHighlight
            visible: moveDropAction.isHovered || copyDropAction.isHovered

            onVisibleChanged: {
                ActivitySwitcher.Backend.setDropMode(visible);
                if (visible) {
                    root.state = "dropAreasShown";
                } else {
                    root.state = "plain";
                }
            }

            anchors {
                fill: parent
                topMargin: icon.height + 3 * PlasmaCore.Units.smallSpacing
            }

            opacity: .75
            color: PlasmaCore.Theme.backgroundColor
        }

        TaskDropArea {
            id: moveDropAction

            anchors {
                right: parent.horizontalCenter
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }

            topPadding: icon.height + 3 * PlasmaCore.Units.smallSpacing
            actionVisible: dropHighlight.visible

            actionTitle: i18nd("plasma_shell_org.kde.plasma.desktop", "Move to\nthis activity")

            onTaskDropped: {
                ActivitySwitcher.Backend.dropMove(mimeData, root.activityId);
            }

            onClicked: {
                root.clicked();
            }

            onEntered: {
                S.showActivityItemActionsBar(root);
            }

            visible: ActivitySwitcher.Backend.dropEnabled
        }

        TaskDropArea {
            id: copyDropAction

            topPadding: icon.height + 3 * PlasmaCore.Units.smallSpacing
            actionVisible: dropHighlight.visible

            anchors {
                right: parent.right
                left: parent.horizontalCenter
                top: parent.top
                bottom: parent.bottom
            }

            actionTitle: i18nd("plasma_shell_org.kde.plasma.desktop", "Show also\nin this activity")

            onTaskDropped: {
                ActivitySwitcher.Backend.dropCopy(mimeData, root.activityId);
            }

            onClicked: {
                root.clicked();
            }

            onEntered: {
                S.showActivityItemActionsBar(root);
            }

            visible: ActivitySwitcher.Backend.dropEnabled

        }

        // Controls
        Item {
            id: controlBar

            height: root.state == "showingControls" ?
                        (configButton.height + 4 * PlasmaCore.Units.smallSpacing) :
                        0

            Behavior on height {
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                }
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: PlasmaCore.Units.shortDuration
                }
            }

            clip: true

            anchors {
                bottom : parent.bottom
                left   : parent.left
                right  : parent.right
            }

            Rectangle {
                anchors {
                    fill: parent
                    margins: - 2 * PlasmaCore.Units.smallSpacing
                }

                opacity: .75
                color: PlasmaCore.Theme.backgroundColor
            }

            PlasmaComponents.Button {
                id: configButton

                iconSource: "configure"
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Configure")

                onClicked: ActivitySettings.configureActivity(root.activityId);

                anchors {
                    left       : parent.left
                    top        : parent.top
                    leftMargin : 2 * PlasmaCore.Units.smallSpacing + 2
                    topMargin  : 2 * PlasmaCore.Units.smallSpacing
                }
            }

            PlasmaComponents.Button {
                id: stopButton

                visible: stoppable
                iconSource: "process-stop"
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Stop activity")

                onClicked: ActivitySwitcher.Backend.stopActivity(activityId);

                anchors {
                    right       : parent.right
                    top         : parent.top
                    rightMargin : 2 * PlasmaCore.Units.smallSpacing + 2
                    topMargin   : 2 * PlasmaCore.Units.smallSpacing
                }
            }
        }
    }

    states: [
        State {
            name: "plain"
            PropertyChanges { target: shade; visible: true }
            PropertyChanges { target: controlBar; opacity: 0 }
        },
        State {
            name: "showingControls"
            PropertyChanges { target: shade; visible: true }
            PropertyChanges { target: controlBar; opacity: 1 }
        },
        State {
            name: "dropAreasShown"
            // PropertyChanges { target: shade; visible: false }
            PropertyChanges { target: statsBar; visible: false }
            PropertyChanges { target: controlBar; opacity: 0 }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                properties : "opacity"
                duration   : PlasmaCore.Units.shortDuration
            }
        }
    ]
}
