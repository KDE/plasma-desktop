/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Shapes

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm
import org.kde.kcmutils
import org.kde.kquickcontrols

Kirigami.FormLayout {
    id: root

    required property var db
    required property var device
    required property var tabletEvents

    readonly property bool calibrationWindowOpen: calibrationWindow !== null
    property Calibration calibrationWindow
    property string currentCalibrationSysName

    signal settingsRestored()

    RowLayout {
        Kirigami.FormData.label: i18nd("kcm_tablet", "Left-handed mode:")
        Kirigami.FormData.buddyFor: leftHandedCheckbox
        spacing: 0

        QQC2.CheckBox {
            id: leftHandedCheckbox
            text: i18nc("Enable left-handed mode", "Enable")
            enabled: root.device && root.device.supportsLeftHanded
            checked: root.device && root.device.leftHanded
            onCheckedChanged: root.device.leftHanded = checked
        }
        Kirigami.ContextualHelpButton {
            toolTipText: xi18nc("@info", "Tells the device to accommodate left-handed users. Effects will vary by device, but often it reverses the pad buttonsʼ functionality so the tablet can be used upside-down.")
        }
        SettingHighlighter {
            // No device has a default of left-handed, so this is always an explicit user choice
            highlight: leftHandedCheckbox.checked
        }
    }

    Repeater {
        model: StylusButtonsModel {
            device: root.device
            db: root.db
        }

        delegate: ActionBinding {
            id: seq

            required property var modelData

            Kirigami.FormData.label: (buttonPressed ? "<b>" : "") + modelData.label + (buttonPressed ? "</b>" : "")
            property bool buttonPressed: false

            name: modelData.name

            Connections {
                target: root.tabletEvents
                function onToolButtonReceived(hardware_serial_hi, hardware_serial_lo, button, pressed) {
                    if (button !== modelData.value) {
                        return;
                    }
                    seq.buttonPressed = pressed
                }
            }

            Connections {
                target: root
                function onSettingsRestored(): void {
                    refreshInputSequence();
                }
            }

            function refreshInputSequence(): void {
                seq.inputSequence = kcm.toolButtonMapping(root.device.name, modelData.value)
            }

            Component.onCompleted: refreshInputSequence()

            onGotInputSequence: sequence => {
                kcm.assignToolButtonMapping(root.device.name, modelData.value, sequence)
            }

            SettingHighlighter {
                // Currently, application-defined is the default
                highlight: seq.inputSequence.type !== InputSequence.ApplicationDefined
            }
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18nd("kcm_tablet", "Pen Pressure:")

        Layout.fillWidth: true

        spacing: Kirigami.Units.smallSpacing

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            PressureCurve {
                id: pressureCurve

                onControlPoint1Changed: saveSettings()
                onControlPoint2Changed: saveSettings()
                isDefault: root.device.pressureCurveIsDefault

                Layout.fillWidth: true

                Component.onCompleted: reloadSettings()

                function reloadSettings(): void {
                    if (!root.device) {
                        return;
                    }

                    const points = kcm.fromSerializedCurve(root.device.pressureCurve);
                    if (points.length === 2) {
                        pressureCurve.controlPoint1 = points[0];
                        pressureCurve.controlPoint2 = points[1];
                        pressureCurve.forceReloadControlPoints();
                    } else {
                        pressureCurve.controlPoint1 = Qt.point(0.0, 0.0);
                        pressureCurve.controlPoint2 = Qt.point(1.0, 1.0);
                    }
                }

                function saveSettings(): void {
                    if (!root.device) {
                        return;
                    }

                    root.device.pressureCurve = kcm.toSerializedCurve(pressureCurve.controlPoint1, pressureCurve.controlPoint2);
                }

                Connections {
                    target: root.device

                    // For reloading the curve when it's reset/set to default
                    function onPressureCurveChanged(): void {
                        pressureCurve.reloadSettings();
                    }
                }
            }
            RowLayout {
                QQC2.Label {
                    text: i18ndc("kcm_tablet", "% of minimum pen pressure", "%1%", Math.round(root.device.pressureRangeMin * 100.0))
                }

                Item {
                    Layout.fillWidth: true
                }

                QQC2.Label {
                    text: i18ndc("kcm_tablet", "% of maximum pen pressure", "%1%", Math.round(root.device.pressureRangeMax * 100.0))
                }
            }
        }
        ColumnLayout {
            Layout.maximumHeight: pressureCurve.implicitHeight
            Layout.alignment: Qt.AlignTop

            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: i18ndc("kcm_tablet", "100% or maximum pen pressure", "100%")
            }

            Item {
                Layout.fillHeight: true
            }

            QQC2.Label {
                text: i18ndc("kcm_tablet", "0% or zero pen pressure", "0%")
            }
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18ndc("kcm_tablet", "@info", "This curve controls the relationship between the pressure on the stylus and the pressure values received by applications.")
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18ndc("kcm_tablet", "Pen pressure range", "Pressure Range:")

        spacing: Kirigami.Units.smallSpacing
        enabled: root.device.supportsPressureRange

        Layout.fillWidth: true

        QQC2.RangeSlider {
            from: 0
            to: 1
            first {
                value: root.device.pressureRangeMin
                onMoved: root.device.pressureRangeMin = first.value
            }
            second {
                value: root.device.pressureRangeMax
                onMoved: root.device.pressureRangeMax = second.value
            }

            Layout.preferredWidth: pressureCurve.width
        }

        Item {
            Layout.fillWidth: true
        }

        Kirigami.ContextualHelpButton {
            toolTipText: i18ndc("kcm_tablet", "@info", "Pressure above or below the threshold will be clamped, and this becomes the new effective range.")
        }
    }


    QQC2.Button {
        // TODO: don't allow calibration across multiple screens again
        readonly property bool supportsCalibration: root.device.supportsCalibrationMatrix

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

                const window = component.createObject(root, {device: root.device, tabletEvents: root.tabletEvents});
                // We need to show the window first, because Qt will override screen based on position.
                // Working around QTBUG-129989
                window.show();
                // Then override the screen, try showing it again and now it'll be on the correct screen:
                window.screen = Qt.application.screens[screenIndex];
                window.showFullScreen();
                window.closing.connect((close) => {
                    root.calibrationWindow = null;
                });

                root.currentCalibrationSysName = root.device.sysName;
                root.calibrationWindow = window;
            }
        }
    }

    ActionDialog {
        id: actionDialog

        parent: root.QQC2.Overlay.overlay
    }
}
