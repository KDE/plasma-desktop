/*   vim:set foldmethod=marker:

    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2

import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.ksvg 1.0 as KSvg
import org.kde.kirigami 2.20 as Kirigami

import org.kde.kcmutils  // KCMLauncher
import org.kde.config  // KAuthorized

import org.kde.plasma.activityswitcher as ActivitySwitcher

import "static.js" as S

Item {
    id: root

    property int innerPadding: Kirigami.Units.smallSpacing

    property string activityId : ""

    property alias title       : title.text
    property alias icon        : icon.source

    signal clicked

    width  : 200
    height : icon.height + 2 * Kirigami.Units.smallSpacing

    // Background until we get something real
    KSvg.FrameSvgItem {
        id: highlight
        imagePath: "widgets/viewitem"
        visible: rootArea.containsMouse

        anchors {
            fill: parent
            rightMargin: -highlight.margins.right
            bottomMargin: -highlight.margins.bottom
        }

        prefix: "normal"
    }

    Item {
        anchors {
            fill: parent

            leftMargin: highlight.margins.left
            topMargin: highlight.margins.top
        }

        // Title and the icon

        Kirigami.Icon {
            id: icon

            width  : Kirigami.Units.iconSizes.medium
            height : width

            anchors {
                left    : parent.left
                margins : root.innerPadding
                verticalCenter: parent.verticalCenter
            }
        }

        PlasmaComponents.Label {
            id: title

            elide : Text.ElideRight

            anchors {
                left    : icon.right
                right   : parent.right
                margins : root.innerPadding * 2
                verticalCenter: parent.verticalCenter
            }
            textFormat: Text.PlainText
        }

        // Controls

        MouseArea {
            id: rootArea

            anchors.fill : parent
            hoverEnabled : true

            Accessible.name          : root.title
            Accessible.role          : Accessible.Button
            Accessible.selectable    : false
            Accessible.onPressAction : root.clicked()

            onClicked    : root.clicked()
            onEntered    : S.showActivityItemActionsBar(root)
        }

        Item {
            id: controlBar

            height: root.state == "showingControls" ?
                        root.height :
                        0

            Behavior on height {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                }
            }

            Behavior on opacity {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration
                }
            }

            clip: true

            anchors {
                left   : parent.left
                right  : parent.right
                bottom : parent.bottom
            }

            PlasmaComponents.Button {
                id: configButton

                icon.name: "configure"
                PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay
                PlasmaComponents.ToolTip.visible: hovered
                PlasmaComponents.ToolTip.text: i18nd("plasma_shell_org.kde.plasma.desktop", "Configure activity")

                onClicked: KCMLauncher.openSystemSettings("kcm_activities", root.activityId)

                anchors {
                    right       : deleteButton.left
                    rightMargin : 2 * Kirigami.Units.smallSpacing
                    verticalCenter: parent.verticalCenter
                }
            }

            PlasmaComponents.Button {
                id: deleteButton

                icon.name: "edit-delete"
                PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay
                PlasmaComponents.ToolTip.visible: hovered
                PlasmaComponents.ToolTip.text: i18nd("plasma_shell_org.kde.plasma.desktop", "Delete")

                onClicked: ActivitySwitcher.Backend.removeActivity(root.activityId)
                visible: KAuthorized.authorize("plasma-desktop/add_activities")

                anchors {
                    right       : parent.right
                    rightMargin : 2 * Kirigami.Units.smallSpacing + 2
                    verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    states: [
        State {
            name: "plain"
            PropertyChanges { target: controlBar; opacity: 0 }
        },
        State {
            name: "showingControls"
            PropertyChanges { target: controlBar; opacity: 1 }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                properties : "opacity"
                duration   : Kirigami.Units.shortDuration
            }
        }
    ]
}


