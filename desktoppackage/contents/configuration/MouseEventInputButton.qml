/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2


QQC2.Button {
    id: mouseInputButton
    property string defaultText: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button inactive state for button recording mouse input" ,"Add Action")
    text: defaultText
    checkable: true
    property string eventString

    onCheckedChanged: {
        if (checked) {
            text = i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button active state for button recording mouse input", "Input Here");
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

            if (mouseInputButton.eventString === newEventString || !configDialog.currentContainmentActionsModel.isTriggerUsed(newEventString)) {
                mouseInputButton.eventString = newEventString;
                mouseInputButton.text = mouseInputButton.defaultText;
                mouseInputButton.checked = false;
                enabled = false;
            }
        }

        onWheel: {
            var newEventString = configDialog.currentContainmentActionsModel.wheelEventString(wheel);

            if (mouseInputButton.eventString === newEventString || !configDialog.currentContainmentActionsModel.isTriggerUsed(newEventString)) {
                mouseInputButton.eventString = newEventString;
                mouseInputButton.text = mouseInputButton.defaultText;
                mouseInputButton.checked = false;
                enabled = false;
            }
        }
    }
}

