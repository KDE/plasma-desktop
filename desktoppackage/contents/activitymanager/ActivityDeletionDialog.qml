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

PlasmaCore.FrameSvgItem {
    id: root

    signal accepted()
    signal canceled()

    imagePath: "dialogs/background"

    property string activityId: ""







    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
    }

    function open(verticalPosition)
    {
        y = verticalPosition - height / 2;
        opacity = 1;
    }

    function close()
    {
        opacity = 0;
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

    height: content.height + margins.top + margins.bottom
    Column {
        id: content

        anchors {
            left: parent.left
            right: parent.right

            topMargin: root.margins.top
            leftMargin: root.margins.left
            bottomMargin: root.margins.bottom
            rightMargin: root.margins.right
        }

        PlasmaComponents.Label {
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Are you sure you want to delete the activity?")
            width: parent.width
        }

        Item {
            height: units.largeSpacing
            width: parent.width
        }

        PlasmaComponents.ButtonRow {
            exclusive: false

            PlasmaComponents.Button {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Delete")
                iconSource: "list-remove"
                onClicked: {
                    root.close();
                    root.accepted();
                }
            }
            PlasmaComponents.Button {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel")
                iconSource: "dialog-close"
                onClicked: {
                    root.close();
                    root.canceled();
                }
            }

        }

    }
}
