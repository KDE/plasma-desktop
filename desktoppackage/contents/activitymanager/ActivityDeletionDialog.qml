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
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import "static.js" as S

PlasmaCore.FrameSvgItem {
    id: root

    signal accepted()
    signal canceled()

    imagePath: "dialogs/background"

    property alias activityName: activityNameText.text
    property alias activityIconSource: icon.source

    property alias acceptButtonText: acceptButton.text
    property alias cancelButtonText: cancelButton.text

    property string activityId: ""

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
    }

    function open(verticalPosition)
    {
        y = verticalPosition - height / 2;
        opacity = 1;
        activityNameText.forceActiveFocus();

        S.openedDialog(root);
    }

    function close()
    {
        opacity = 0;

        S.closedDialog(root);
    }

    opacity: 0
    visible: false
    Behavior on opacity { PropertyAnimation { duration: units.shortDuration } }
    Behavior on y { PropertyAnimation {
            duration: root.visible ? units.shortDuration : 0
    } }
    onOpacityChanged: visible = (opacity > 0)

    PlasmaCore.FrameSvgItem {
        id: shadowFrame

        imagePath: "dialogs/background"
        prefix: "shadow"

        z: -1

        anchors {
            fill: parent

            leftMargin   : -margins.left
            topMargin    : -margins.top
            rightMargin  : -margins.right
            bottomMargin : -margins.bottom
        }
    }

    height: content.height + margins.top + margins.bottom + dialogButtons.height

    Item {
        id: content

        anchors {
            left  : parent.left
            right : parent.right
            top   : parent.top

            topMargin    : units.smallSpacing + root.margins.top
            leftMargin   : units.smallSpacing + root.margins.left
            rightMargin  : units.smallSpacing + root.margins.right
        }

        PlasmaCore.IconItem {
            id: icon

            anchors {
                left: parent.left
                top: parent.top
            }

            width: height
            height: units.iconSizes.medium
        }

        PlasmaComponents.Label {
            id: activityNameText

            anchors {
                left:  icon.right
                top:   parent.top
                right: parent.right

                leftMargin: units.largeSpacing
            }
        }

        PlasmaComponents.Label {
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Are you sure you want to delete this activity?")
            width: parent.width

            anchors {
                top: icon.bottom
                left: parent.left
                right: parent.right
            }
        }
    }

    PlasmaComponents.ButtonRow {
        id: dialogButtons

        anchors {
            right:  parent.right
            bottom: parent.bottom
            rightMargin:  root.margins.right
            bottomMargin: root.margins.bottom
        }

        exclusive: false

        PlasmaComponents.Button {
            id: acceptButton

            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Delete")
            enabled: activityNameText.text.trim().length > 0
            iconSource: "list-add"
            onClicked: {
                root.close();
                root.accepted();
            }
        }

        PlasmaComponents.Button {
            id: cancelButton

            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel")
            iconSource: "dialog-close"
            onClicked: {
                root.close();
                root.canceled();
            }
        }
    }
}
