/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 2.3 as QQC2


QQC2.Button {
    id: mouseInputButton
    property string defaultText: i18nd("plasma_shell_org.kde.plasma.desktop", "Add Action")
    text: defaultText
    checkable: true
    property string eventString

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
            var newEventString = configDialog.currentContainmentActionsModel.wheelEventString(wheel);

            if (eventString === newEventString || !configDialog.currentContainmentActionsModel.isTriggerUsed(newEventString)) {
                eventString = newEventString;
                mouseInputButton.text = defaultText;
                mouseInputButton.checked = false;
                enabled = false;
            }
        }
    }
}

