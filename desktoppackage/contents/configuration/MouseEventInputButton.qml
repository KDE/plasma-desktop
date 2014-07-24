/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Layouts 1.0
import org.kde.plasma.core 2.0 as PlasmaCore


QtControls.Button {
    id: mouseInputButton
    property string defaultText: i18nd("plasma_shell_org.kde.plasma.desktop", "Add Action")
    text: defaultText
    checkable: true
    property string eventString

    implicitWidth: theme.mSize(theme.defaultFont).width * 15
    Layout.minimumWidth: implicitWidth
    Layout.maximumWidth: implicitWidth

    onCheckedChanged: {
        if (checked) {
            text = i18nd("plasma_shell_org.kde.plasma.desktop", "Input Here");
            mouseInputArea.enabled = true;
        }
    }
    MouseArea {
        id: mouseInputArea
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        enabled: false

        onClicked: {
            var newEventString = configDialog.currentContainmentActionsModel.mouseEventString(mouse.button, mouse.modifiers);

            if (eventString === newEventString || !configDialog.currentContainmentActionsModel.isTriggerUsed(newEventString)) {
                eventString = newEventString;
                mouseInputButton.text = defaultText;
                mouseInputButton.checked = false;
                enabled = false;
            }
        }

        onWheel: {
            var newEventString = configDialog.currentContainmentActionsModel.wheelEventString(wheel.pixelDelta, wheel.buttons, wheel.modifiers);

            if (eventString === newEventString || !configDialog.currentContainmentActionsModel.isTriggerUsed(newEventString)) {
                eventString = newEventString;
                mouseInputButton.text = defaultText;
                mouseInputButton.checked = false;
                enabled = false;
            }
        }
    }
}

