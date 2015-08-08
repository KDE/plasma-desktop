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

import "static.js" as S

Item {
    id: root

    property int innerPadding: units.smallSpacing

    property string activityId : ""

    property alias title       : title.text
    property alias icon        : icon.source

    property bool showingDialog: configureDialog.itemVisible || deleteDialog.itemVisible

    signal clicked

    width  : 200

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

            onClicked    : root.clicked()
        }

        ControlButton {
            id: deleteButton

            onClicked: S.openActivityDeletionDialog(
                            deleteDialog,
                            root.activityId,
                            root.title,
                            root.icon,
                            // We need to pass some QML-only variables
                            {
                                kactivities: activitiesModel,
                                readyStatus: Loader.Ready,
                                i18nd:       i18nd
                            }
                        )

            icon: "delete"

            anchors {
                right   : parent.right
                top     : parent.top
                margins : root.innerPadding
            }
        }

        ControlButton {
            id: configButton

            onClicked: S.openActivityConfigurationDialog(
                            configureDialog,
                            root.activityId,
                            root.title,
                            root.icon,
                            // We need to pass some QML-only variables
                            {
                                kactivities: activitiesModel,
                                readyStatus: Loader.Ready,
                                i18nd:       i18nd
                            }
                        )

            icon: "configure"

            anchors {
                left    : parent.left
                top     : parent.top
                margins : root.innerPadding
            }
        }

        Loader {
            id: deleteDialog

            anchors.fill: parent

            property bool itemVisible: status == Loader.Ready && item.visible
        }

        Loader {
            id: configureDialog

            anchors.fill: parent

            property bool itemVisible: status == Loader.Ready && item.visible
        }

        states: [
            State {
                name: "plain"
                PropertyChanges { target: deleteButton ; opacity: 0 }
                PropertyChanges { target: configButton ; opacity: 0 }
                PropertyChanges { target: icon         ; visible: true }
                PropertyChanges { target: title        ; visible: true }
                PropertyChanges { target: root         ; height: icon.height + innerPadding }
            },
            State {
                name: "showingControls"
                PropertyChanges { target: deleteButton ; opacity: 1 }
                PropertyChanges { target: configButton ; opacity: 1 }
                PropertyChanges { target: icon         ; visible: true }
                PropertyChanges { target: title        ; visible: true }
                PropertyChanges { target: root         ; height: icon.height + innerPadding }
            },
            State {
                name: "showingOverlayDialog"
                PropertyChanges { target: deleteButton ; opacity: 0 }
                PropertyChanges { target: configButton ; opacity: 0 }
                PropertyChanges { target: icon         ; visible: false }
                PropertyChanges { target: title        ; visible: false }
                PropertyChanges { target: root         ; height: width * 9.0 / 16.0 }
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

        state:
            root.showingDialog          ? "showingOverlayDialog"
            : rootArea.containsMouse    ? "showingControls"
            : /* otherwise */             "plain"
    }

    Behavior on height { PropertyAnimation { duration: units.shortDuration } }
}


