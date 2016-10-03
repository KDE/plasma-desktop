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

    property int innerPadding  : units.largeSpacing

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

    onBackgroundChanged: if (background[0] != '#') {
        // We have a proper wallpaper, hurroo!
        backgroundColor.visible = false;

    } else {
        // We have only a color
        backgroundColor.color = background;
        backgroundColor.visible = true;
    }

    signal clicked

    width  : 200
    // height : width * 1 / units.displayAspectRatio
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
            id: highlight

            visible:  root.current

            border.width: root.current ? units.smallSpacing : 0
            border.color: theme.highlightColor

            anchors {
                fill: parent
                // Hide the rounding error on the bottom of the rectangle
                bottomMargin: -1
            }

            color: "transparent"

            // z: 1
        }

        Item {
            id: titleBar

            anchors {
                top   : parent.top
                left  : parent.left
                right : parent.right

                leftMargin : 2 * units.smallSpacing + 2
                topMargin  : 2 * units.smallSpacing
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

                width   : units.iconSizes.medium
                height  : width

                anchors {
                    right       : parent.right
                    rightMargin : 2 * units.smallSpacing
                }
            }
        }

        Column {
            id: statsBar

            height: childrenRect.height + units.smallSpacing

            anchors {
                bottom : controlBar.top
                left   : parent.left
                right  : parent.right

                leftMargin   : 2 * units.smallSpacing + 2
                rightMargin  : 2 * units.smallSpacing
                bottomMargin : units.smallSpacing
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

            // Text {
            //     id: stats
            //
            //     color   : "white"
            //     elide   : Text.ElideRight
            //     opacity : .6
            //
            //     text: "6 documents, 2 applications"
            //     visible: false
            //
            //     anchors {
            //         top   : lastUsedDate.bottom
            //         left  : parent.left
            //         right : parent.right
            //     }
            // }
        }

        MouseArea {
            id: hoverArea

            anchors.fill : parent
            onClicked    : root.clicked()
            hoverEnabled : true
            onEntered    : S.showActivityItemActionsBar(root)

            Accessible.name          : root.title
            Accessible.role          : Accessible.Button
            Accessible.selected      : root.selected
            Accessible.onPressAction : root.clicked()
        }

        // Controls
        Item {
            id: controlBar

            height: root.state == "showingControls" ?
                        (configButton.height + 4 * units.smallSpacing) :
                        0

            Behavior on height {
                NumberAnimation {
                    duration: units.longDuration
                }
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: units.shortDuration
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
                    margins: - 2 * units.smallSpacing
                }

                opacity: .75
                color: theme.backgroundColor
            }

            PlasmaComponents.Button {
                id: configButton

                iconSource: "configure"
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Configure")

                onClicked: ActivitySettings.configureActivity(root.activityId);

                anchors {
                    left       : parent.left
                    top        : parent.top
                    leftMargin : 2 * units.smallSpacing + 2
                    topMargin  : 2 * units.smallSpacing
                }
            }

            PlasmaComponents.Button {
                id: stopButton

                iconSource: "process-stop"
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Stop activity")

                onClicked: ActivitySwitcher.Backend.stopActivity(activityId);

                anchors {
                    right       : parent.right
                    top         : parent.top
                    rightMargin : 2 * units.smallSpacing + 2
                    topMargin   : 2 * units.smallSpacing
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
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                properties : "opacity"
                duration   : units.shortDuration
            }
        }
    ]
}
