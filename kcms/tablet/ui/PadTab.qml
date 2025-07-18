/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Shapes

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm as KCM
import org.kde.kcmutils
import org.kde.kquickcontrols

Kirigami.FormLayout {
    id: root

    required property KCM.InputDevice device
    required property KCM.InputDevice padDevice
    required property KCM.TabletEvents tabletEvents

    Repeater {
        id: dialRepeater
        model: root.padDevice.tabletPadDialCount

        delegate: ActionBinding {
            id: seq

            required property int index

            Kirigami.FormData.label: i18nd("kcm_tablet", "Pad dial %1:", index + 1)

            name: i18ndc("kcm_tablet", "@info Meant to be inserted into an existing sentence like 'configuring pad button 0'", "pad dial %1", index + 1)
            supportsPenButton: false
            supportsRelativeEvents: true

            function refreshInputSequence(): void {
                seq.inputSequence = kcm.padDialMapping(root.padDevice.name, index)
            }

            inputSequence: kcm.padDialMapping(root.padDevice.name, index)
            Connections {
                target: kcm
                function onSettingsRestored() {
                    refreshInputSequence();
                }
            }

            onGotInputSequence: sequence => {
                kcm.assignPadDialMapping(root.padDevice.name, index, sequence)
            }

            SettingHighlighter {
                // Currently, application-defined is the default
                highlight: seq.inputSequence.type !== KCM.InputSequence.ApplicationDefined
            }
        }
    }

    Repeater {
        id: buttonsRepeater
        model: root.padDevice.tabletPadButtonCount

        delegate: ActionBinding {
            id: seq

            required property int index

            Kirigami.FormData.label: (buttonPressed ? "<b>" : "") + i18nd("kcm_tablet", "Pad button %1:", index + 1) + (buttonPressed ? "</b>" : "")
            property bool buttonPressed: false

            name: i18ndc("kcm_tablet", "@info Meant to be inserted into an existing sentence like 'configuring pad button 0'", "pad button %1", index + 1)
            supportsPenButton: false

            Connections {
                target: root.tabletEvents
                function onPadButtonReceived(path, button, pressed) {
                    if (button !== index || !path.endsWith(root.padDevice.sysName)) {
                        return;
                    }
                    seq.buttonPressed = pressed
                }
            }

            function refreshInputSequence(): void {
                seq.inputSequence = kcm.padButtonMapping(root.padDevice.name, index)
            }

            inputSequence: kcm.padButtonMapping(root.padDevice.name, index)
            Connections {
                target: kcm
                function onSettingsRestored() {
                    refreshInputSequence();
                }
            }

            onGotInputSequence: sequence => {
                kcm.assignPadButtonMapping(root.padDevice.name, index, sequence)
            }

            SettingHighlighter {
                // Currently, application-defined is the default
                highlight: seq.inputSequence.type !== KCM.InputSequence.ApplicationDefined
            }
        }
    }

    ActionDialog {
        id: actionDialog

        parent: root.QQC2.Overlay.overlay
    }
}
