/*   vim:set foldmethod=marker:
 *
 *   Copyright (C) 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.2

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.activities 0.1 as Activities
import org.kde.activities.settings 0.1

import "static.js" as S

Item {
    id: root

    property int innerPadding: units.smallSpacing

    property string activityId : ""

    property alias title       : title.text
    property alias icon        : icon.source

    signal clicked

    width  : 200
    height : icon.height + 2 * units.smallSpacing

    // Background until we get something real
    PlasmaCore.FrameSvgItem {
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

        PlasmaCore.IconItem {
            id: icon

            width  : units.iconSizes.medium
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
                left   : parent.left
                right  : parent.right
                bottom : parent.bottom
            }

            PlasmaComponents.Button {
                id: configButton

                iconSource: "configure"
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Configure activity")

                onClicked: ActivitySettings.configureActivity(root.activityId)

                anchors {
                    left       : parent.left
                    leftMargin : 2 * units.smallSpacing + 2
                    verticalCenter: parent.verticalCenter
                }
            }

            PlasmaComponents.Button {
                id: deleteButton

                iconSource: "edit-delete"
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Delete")

                onClicked: ActivitySettings.deleteActivity(root.activityId)
                visible: ActivitySettings.newActivityAuthorized

                anchors {
                    right       : parent.right
                    rightMargin : 2 * units.smallSpacing + 2
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
                duration   : units.shortDuration
            }
        }
    ]
}


