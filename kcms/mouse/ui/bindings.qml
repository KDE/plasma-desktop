/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kcmutils as KCMUtils

import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols as KQuickControls

import org.kde.plasma.private.kcm_mouse as Mouse

KCMUtils.SimpleKCM {
    id: root

    readonly property Mouse.KWinWaylandBackend backend: KCMUtils.ConfigModule.inputBackend

    title: i18ndc("kcmmouse", "@title", "Extra Mouse Buttons")

    header: Header {
        saveLoadMessage: root.KCMUtils.ConfigModule.saveLoadMessage
        hotplugMessage: root.KCMUtils.ConfigModule.hotplugMessage
    }

    MouseArea {
        // Deliberately using MouseArea on the page instead of a TapHandler on the button, so we can capture clicks anywhere
        id: buttonCapture

        property var lastButton: undefined

        parent: root.contentItem
        anchors.fill: parent
        enabled: newBinding.checked
        preventStealing: true
        acceptedButtons: Qt.AllButtons & ~(Qt.LeftButton | Qt.RightButton | Qt.MiddleButton)

        onClicked: mouse => {
            lastButton = buttonMappings.extraButtons.find(entry => Qt[entry.buttonName] === mouse.button)
            newBinding.visible = false
            newKeySequenceItem.visible = true
            newKeySequenceItem.startCapturing()
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.FormLayout {
            id: buttonLayout

            twinFormLayouts: otherLayout

            Repeater {
                id: buttonMappings

                model: extraButtons.filter(entry => entry.buttonName in root.backend.buttonMapping) ?? []

                readonly property var extraButtons: Array.from({length: 24}, (value, index) => ({
                    buttonName: "ExtraButton" + (index + 1),
                    button: Qt["ExtraButton" + (index + 1)],
                    label: i18ndc("kcmmouse", "@label for assigning an action to a numbered button", "Extra Button %1:", index + 1)
                }))

                delegate: KQuickControls.KeySequenceItem {
                    required property var modelData

                    Kirigami.FormData.label: modelData.label

                    keySequence: root.backend.buttonMapping[modelData.buttonName]

                    modifierlessAllowed: true
                    modifierOnlyAllowed: true
                    multiKeyShortcutsAllowed: false
                    checkForConflictsAgainst: KQuickControls.ShortcutType.None

                    onCaptureFinished: {
                        const copy = root.backend.buttonMapping;
                        copy[modelData.buttonName] = keySequence
                        root.backend.buttonMapping = copy
                    }
                }
            }
        }

        Kirigami.InlineMessage {
            id: explanationLabel
            Layout.fillWidth: true
            visible: newBinding.checked || newKeySequenceItem.visible

            text: newBinding.visible
                ? i18ndc("kcmmouse","@action:button", "Press the mouse button for which you want to add a key binding")
                : i18ndc("kcmmouse","@action:button, %1 is the translation of 'Extra Button %1' from above",
                    "Enter the new key combination for %1", buttonCapture.lastButton?.label ?? "")

            actions: Kirigami.Action {
                icon.name: "dialog-cancel"
                text: i18ndc("kcmmouse", "@action:button", "Cancel")
                onTriggered: source => {
                    newKeySequenceItem.visible = false;
                    newBinding.visible = true
                    newBinding.checked = false
                }
            }
        }

        Kirigami.FormLayout {
            id: otherLayout

            twinFormLayouts: buttonLayout

            QQC2.Button {
                id: newBinding
                checkable: true
                text: checked
                    ? i18ndc("kcmmouse", "@action:button", "Press a mouse button")
                    : i18ndc("kcmmouse", "@action:button, Bind a mousebutton to keyboard key(s)", "Add Binding…")
                icon.name: "list-add"
            }

            KQuickControls.KeySequenceItem {
                id: newKeySequenceItem
                visible: false

                modifierlessAllowed: true
                modifierOnlyAllowed: true
                multiKeyShortcutsAllowed: false
                checkForConflictsAgainst: KQuickControls.ShortcutType.None

                onCaptureFinished: {
                    visible = false
                    newBinding.visible = true
                    newBinding.checked = false
                    const copy = root.backend.buttonMapping;
                    copy[buttonCapture.lastButton.buttonName] = keySequence
                    root.backend.buttonMapping = copy
                }
            }
        }
    }
}
