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
    property bool calibrationWindowOpen: false

    QQC2.Button {
        // The device must support calibration, and we don't support calibration across multiple screens
        readonly property bool supportsCalibration: root.device.supportsCalibrationMatrix && !root.device.mapToWorkspace

        text: {
            if (supportsCalibration) {
                if (root.calibrationWindowOpen) {
                    return i18nc("@action:button Calibration in progress", "Calibration in Progress");
                } else {
                    return i18nc("@action:button Calibrate the pen display", "Calibrate");
                }
            } else {
                return i18nc("@action:button Pen display doesn't support calibration", "Calibration Not Supported");
            }
        }
        icon.name: "crosshairs"
        enabled: supportsCalibration && !root.calibrationWindowOpen
        onClicked: {
            const component = Qt.createComponent("Calibration.qml");
            if (component.status === Component.Ready) {
                let screenIndex = 0;
                for (let i = 0; i < Qt.application.screens.length; i++) {
                    if (Qt.application.screens[i].name === root.device.outputName) {
                        screenIndex = i;
                        break;
                    }
                }

                const window = component.createObject(root, {device: root.device, tabletEvents, screen: Qt.application.screens[screenIndex]});
                window.showFullScreen();
                window.closing.connect((close) => {
                    root.calibrationWindowOpen = false;
                });

                root.calibrationWindowOpen = true;
            }
        }

        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        QQC2.ToolTip.text: i18nc("@info:tooltip", "Calibration across multiple screens is not supported.")
        QQC2.ToolTip.visible: hovered && !supportsCalibration
    }

    Repeater {
        model: [
            { value: 0x14b, text: i18nd("kcm_tablet", "Pen button 1:") },
            { value: 0x14c, text: i18nd("kcm_tablet", "Pen button 2:") },
            { value: 0x149, text: i18nd("kcm_tablet", "Pen button 3:") }
        ] // BTN_STYLUS, BTN_STYLUS2, BTN_STYLUS3

        delegate: KeySequenceItem {
            id: seq
            Kirigami.FormData.label: (pressed ? "<b>" : "") + modelData.text + (pressed ? "</b>" : "")
            property bool pressed: false

            Connections {
                target: tabletEvents
                function onToolButtonReceived(hardware_serial_hi, hardware_serial_lo, button, pressed) {
                    if (button !== modelData.value) {
                        return;
                    }
                    seq.pressed = pressed
                }
            }

            keySequence: kcm.toolButtonMapping(root.device.name, modelData.value)
            Connections {
                target: kcm
                function onSettingsRestored() {
                    seq.keySequence = kcm.toolButtonMapping(root.device.name, modelData.value)
                }
            }

            showCancelButton: true
            modifierlessAllowed: true
            modifierOnlyAllowed: true
            multiKeyShortcutsAllowed: false
            checkForConflictsAgainst: ShortcutType.None

            onCaptureFinished: {
                kcm.assignToolButtonMapping(root.device.name, modelData.value, keySequence)
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