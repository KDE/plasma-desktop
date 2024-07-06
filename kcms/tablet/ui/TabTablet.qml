/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm
import org.kde.kcmutils
import org.kde.kquickcontrols

Kirigami.FormLayout {
    id: root

    required property var device

    QQC2.ComboBox {
        Kirigami.FormData.label: i18ndc("kcm_tablet", "@label:listbox The pad we are configuring", "Pad:")
        model: kcm.padsModel

        onCurrentIndexChanged: {
            parent.padDevice = kcm.padsModel.deviceAt(currentIndex)
        }

        onCountChanged: if (count > 0 && currentIndex < 0) {
            currentIndex = 0;
        }
        enabled: count > 0
        displayText: enabled ? currentText : i18n("None")
    }

    Repeater {
        id: buttonsRepeater
        model: tabletEvents.padButtons

        delegate: KeySequenceItem {
            id: seq
            Kirigami.FormData.label: (pressed ? "<b>" : "") + i18nd("kcm_tablet", "Pad button %1:", modelData + 1) + (pressed ? "</b>" : "")
            property bool pressed: false

            Connections {
                target: tabletEvents
                function onPadButtonReceived(path, button, pressed) {
                    if (button !== modelData || !path.endsWith(form.padDevice.sysName)) {
                        return;
                    }
                    seq.pressed = pressed
                }
            }

            keySequence: kcm.padButtonMapping(form.padDevice.name, modelData)
            Connections {
                target: kcm
                function onSettingsRestored() {
                    seq.keySequence = kcm.padButtonMapping(form.padDevice.name, modelData)
                }
            }

            showCancelButton: true
            modifierlessAllowed: true
            modifierOnlyAllowed: true
            multiKeyShortcutsAllowed: false
            checkForConflictsAgainst: ShortcutType.None

            onCaptureFinished: {
                kcm.assignPadButtonMapping(form.padDevice.name, modelData, keySequence)
            }
        }
    }

    TabletEvents {
        id: tabletEvents

        anchors.fill: parent

        onPadButtonsChanged: {
            if (!path.endsWith(form.padDevice.sysName)) {
                return;
            }
            buttonsRepeater.model = buttonCount
        }
    }
}