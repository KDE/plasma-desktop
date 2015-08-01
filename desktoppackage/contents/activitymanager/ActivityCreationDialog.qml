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
import org.kde.kquickcontrolsaddons 2.0 as KQuickControls
import org.kde.plasma.platformcomponents 2.0 as PlasmaPlatformComponents
import org.kde.activities 0.1 as Activities

PlasmaCore.FrameSvgItem {
    id: root

    signal accepted()
    signal canceled()

    imagePath: "dialogs/background"

    property alias activityName: activityNameText.text
    property alias activityIconSource: activityIconButton.iconSource

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

    height: content.height + margins.top + margins.bottom + dialogButtons.height

    Item {
        id: content

        anchors {
            left  : parent.left
            right : parent.right
            top   : parent.top

            topMargin    : root.margins.top
            leftMargin   : root.margins.left
            bottomMargin : root.margins.bottom
            rightMargin  : root.margins.right
        }

        PlasmaComponents.Button {
            id: activityIconButton

            anchors {
                left: parent.left
                top: parent.top
            }

            width: height
            height: activityNameLabel.height + activityNameText.height

            KQuickControls.IconDialog {
                id: iconDialog
                onIconNameChanged: activityIconButton.iconSource = iconName
            }

            onClicked: {
                iconDialog.open();
            }

        }

        height: activityNamePanel.height

        Column {
            id: activityNamePanel
            anchors {
                left:  activityIconButton.right
                top:   parent.top
                right: parent.right

                leftMargin: units.largeSpacing
            }

            PlasmaComponents.Label {
                id: activityNameLabel
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Activity name:")

                elide: Text.ElideRight

                width: parent.width
            }

            PlasmaComponents.TextField {
                id: activityNameText

                width:  parent.width

                onAccepted: acceptButton.clicked()
                Keys.onPressed: {
                    if (event.key === Qt.Key_Escape) {
                        cancelButton.clicked()
                    }
                }

                // text: ""
            }

            Item {
                height: units.largeSpacing
                width: parent.width
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

            text: i18nd("plasma_shell_org.kde.plasma.desktop", "Create")
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
