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

SimpleKCM {
    id: root

    property bool testerWindowOpen: false
    readonly property bool calibrationWindowOpen: calibrationWindow !== null
    property Calibration calibrationWindow
    property string currentCalibrationSysName

    ConfigModule.buttons: ConfigModule.Default | ConfigModule.Apply

    implicitWidth: Kirigami.Units.gridUnit * 38
    implicitHeight: Kirigami.Units.gridUnit * 35

    // So it doesn't scroll while dragging
    flickable.interactive: Kirigami.Settings.hasTransientTouchInput

    Kirigami.PlaceholderMessage {
        icon.name: "preferences-desktop-tablet"
        text: i18nd("kcm_tablet", "No drawing tablets found")
        explanation: i18n("Connect a drawing tablet")
        anchors.centerIn: parent
        visible: combo.count === 0
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }

    actions: [
        Kirigami.Action {
            text: i18ndc("kcm_tablet", "Tests tablet functionality like the pen", "Test Tablet…")
            icon.name: "tool_pen-symbolic"
            enabled: !root.testerWindowOpen && combo.count !== 0
            onTriggered: {
                const component = Qt.createComponent("Tester.qml");
                if (component.status === Component.Ready) {
                    const window = component.createObject(root, {tabletEvents});
                    window.showNormal();
                    window.closing.connect((close) => {
                        root.testerWindowOpen = false;
                    });

                    root.testerWindowOpen = true;
                } else {
                    console.error(component.errorString());
                }
            }
        }
    ]

    function cancelCalibration(): void {
        if (root.calibrationWindowOpen) {
            calibrationWindow.close();
            calibrationWindow = null;
        }
    }

    Connections {
        target: combo.model

        function onDeviceRemoved(sysname: string): void {
            if (root.currentCalibrationSysName === sysname) {
                cancelCalibration();
            }
        }
    }

    Kirigami.FormLayout {
        id: form
        visible: combo.count > 0
        enabled: combo.count > 0
        QQC2.ComboBox {
            id: combo
            Kirigami.FormData.label: i18ndc("kcm_tablet", "@label:listbox The device we are configuring", "Device:")
            model: kcm.tabletsModel

            onCountChanged: if (count > 0 && currentIndex < 0) {
                currentIndex = 0;
            }

            function reloadOutputView() {
                const initialOutputArea = form.device.outputArea;
                if (initialOutputArea === Qt.rect(0, 0, 1, 1)) {
                    outputAreaCombo.currentIndex = 0;
                } else if (initialOutputArea.x === 0 && initialOutputArea.y === 0 && initialOutputArea.width === 1) {
                    outputAreaCombo.currentIndex = 1;
                } else {
                    outputAreaCombo.currentIndex = 2;
                }
                if (outputsCombo.currentIndex !== 0) {
                    mapping.setOutputAreaMode(outputAreaCombo.currentIndex);
                    mapping.setOutputArea(initialOutputArea);
                }
            }

            onCurrentIndexChanged: {
                parent.device = kcm.tabletsModel.penAt(combo.currentIndex);
                parent.padDevice = kcm.tabletsModel.padAt(combo.currentIndex);
                reloadOutputView()
                pressureCurve.reloadSettings();
            }

            Connections {
                target: kcm
                function onSettingsRestored() {
                    mapping.changed = false
                    combo.reloadOutputView()
                }
            }
        }

        property QtObject device: null

        QQC2.ComboBox {
            id: outputsCombo
            Kirigami.FormData.label: i18nd("kcm_tablet", "Map to screen:")
            model: OutputsModel {
                id: outputsModel
            }
            currentIndex: {

                if (count === 0) {
                    return -1
                }

                outputsModel.rowForDevice(parent.device)
            }
            textRole: "display"
            onActivated: index => {
                parent.device.outputName = outputsModel.outputNameAt(index)
                parent.device.mapToWorkspace = outputsModel.isMapToWorkspaceAt(index)
                // If we are switching to "Follow current screen", ensure we're force switched to "Stretch to Fill"
                if (index === 0) {
                    mapping.setOutputAreaMode(0);
                    outputAreaCombo.currentIndex = 0;
                }
                combo.reloadOutputView();
            }
        }
        QQC2.ComboBox {
            Kirigami.FormData.label: i18nd("kcm_tablet", "Orientation:")
            model: OrientationsModel {
                id: orientationsModel
            }
            enabled: parent.device && parent.device.supportsOrientation
            currentIndex: orientationsModel.rowForOrientation(parent.device.orientation)
            displayText: parent.device.supportsOrientation ? currentText : i18nd("kcm_tablet", "Not Applicable")
            textRole: "display"
            onActivated: {
                parent.device.orientation = orientationsModel.orientationAt(currentIndex)
            }
            SettingHighlighter {
                // First index is "Default"
                highlight: form.device.supportsOrientation && parent.currentIndex !== 0
            }
        }
        RowLayout {
            Kirigami.FormData.label: i18nd("kcm_tablet", "Left-handed mode:")
            Kirigami.FormData.buddyFor: leftHandedCheckbox
            spacing: 0

            QQC2.CheckBox {
                id: leftHandedCheckbox
                enabled: form.device && form.device.supportsLeftHanded
                checked: form.device && form.device.leftHanded
                onCheckedChanged: form.device.leftHanded = checked
            }
            Kirigami.ContextualHelpButton {
                toolTipText: xi18nc("@info", "Tells the device to accommodate left-handed users. Effects will vary by device, but often it reverses the pad buttonsʼ functionality so the tablet can be used upside-down.")
            }
            SettingHighlighter {
                // No device has a default of left-handed, so this is always an explicit user choice
                highlight: leftHandedCheckbox.checked
            }
        }
        QQC2.ComboBox {
            id: outputAreaCombo
            Layout.fillWidth: true
            Kirigami.FormData.label: i18nd("kcm_tablet", "Mapped Area:")
            enabled: outputsCombo.currentIndex !== 0
            model: outputsCombo.currentIndex === 0 ?
                // We don't support mapping to a portion of the screen with follow current screen, since we have no way to visualize that
                [i18nc("@item:inlistbox Stretch and fill to the screen", "Stretch and Fill")] :
                [i18nc("@item:inlistbox Stretch and fill to the screen", "Stretch and Fill"),
                 i18nc("@item:inlistbox Keep aspect ratio and fit within the screen", "Keep Aspect Ratio and Fit"),
                 i18nc("@item:inlistbox Map to portion of the screen", "Map to Portion")]
            onActivated: index => {
                mapping.changed = true;
                mapping.setOutputAreaMode(index);
            }
            SettingHighlighter {
                // The default is stretch to screen
                highlight: outputAreaCombo.currentIndex !== 0
            }
        }

        // Mapping a tablet onto another display
        ExternalScreenMapping {
            id: mapping
            
            visible: form.device && outputsCombo.currentIndex !== 0
            device: form.device
            mode: outputAreaCombo.currentIndex

            Layout.fillWidth: true
        }

        Repeater {
            model: StylusButtonsModel {
                device: form.device
                db: kcm.db
            }

            delegate: ActionBinding {
                id: seq

                required property var modelData

                Kirigami.FormData.label: (buttonPressed ? "<b>" : "") + modelData.label + (buttonPressed ? "</b>" : "")
                property bool buttonPressed: false

                name: modelData.name

                Connections {
                    target: tabletEvents
                    function onToolButtonReceived(hardware_serial_hi, hardware_serial_lo, button, pressed) {
                        if (button !== modelData.value) {
                            return;
                        }
                        seq.buttonPressed = pressed
                    }
                }

                Connections {
                    target: kcm
                    function onSettingsRestored() {
                        refreshInputSequence();
                    }
                }

                Connections {
                    target: form
                    function onDeviceChanged() {
                        refreshInputSequence();
                    }
                }

                function refreshInputSequence(): void {
                    seq.inputSequence = kcm.toolButtonMapping(form.device.name, modelData.value)
                }

                onGotInputSequence: sequence => {
                    kcm.assignToolButtonMapping(form.device.name, modelData.value, sequence)
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
                    isDefault: form.device.pressureCurveIsDefault

                    Layout.fillWidth: true

                    Component.onCompleted: reloadSettings()

                    function reloadSettings(): void {
                        if (!form.device) {
                            return;
                        }

                        const points = kcm.fromSerializedCurve(form.device.pressureCurve);
                        if (points.length === 2) {
                            pressureCurve.controlPoint1 = points[0];
                            pressureCurve.controlPoint2 = points[1];
                            pressureCurve.forceReloadControlPoints();
                        }
                    }

                    function saveSettings(): void {
                        if (!form.device) {
                            return;
                        }

                        form.device.pressureCurve = kcm.toSerializedCurve(pressureCurve.controlPoint1, pressureCurve.controlPoint2);
                    }

                    Connections {
                        target: form.device

                        // For reloading the curve when it's reset/set to default
                        function onPressureCurveChanged(): void {
                            pressureCurve.reloadSettings();
                        }
                    }
                }
                RowLayout {
                    QQC2.Label {
                        text: i18ndc("kcm_tablet", "Low pen pressure", "Low Pressure")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    QQC2.Label {
                        text: i18ndc("kcm_tablet", "High pen pressure", "High Pressure")
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

        QQC2.Button {
            // The device must support calibration, and we don't support calibration across multiple screens
            readonly property bool supportsCalibration: form.device.supportsCalibrationMatrix && !outputsModel.isMapToWorkspaceAt(outputsCombo.currentIndex)

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
                        if (Qt.application.screens[i].name === form.device.outputName) {
                            screenIndex = i;
                            break;
                        }
                    }

                    const window = component.createObject(root, {device: form.device, tabletEvents});
                    // We need to show the window first, because Qt will override screen based on position.
                    // Working around QTBUG-129989
                    window.show();
                    // Then override the screen, try showing it again and now it'll be on the correct screen:
                    window.screen = Qt.application.screens[screenIndex];
                    window.showFullScreen();
                    window.closing.connect((close) => {
                        root.calibrationWindow = null;
                    });

                    root.currentCalibrationSysName = form.device.sysName;
                    root.calibrationWindow = window;
                }
            }

            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.text: i18nc("@info:tooltip", "Calibration across multiple screens is not supported.")
            QQC2.ToolTip.visible: hovered && outputsModel.isMapToWorkspaceAt(outputsCombo.currentIndex)
        }

        Kirigami.Separator {
            Layout.fillWidth: true
            visible: form.padDevice.tabletPadButtonCount > 0
        }

        property QtObject padDevice: null

        TabletEvents {
            id: tabletEvents

            anchors.fill: parent
        }

        Repeater {
            id: buttonsRepeater
            model: form.padDevice.tabletPadButtonCount

            delegate: ActionBinding {
                id: seq

                required property var modelData

                Kirigami.FormData.label: (buttonPressed ? "<b>" : "") + i18nd("kcm_tablet", "Pad button %1:", modelData + 1) + (buttonPressed ? "</b>" : "")
                property bool buttonPressed: false

                name: i18ndc("kcm_tablet", "@info Meant to be inserted into an existing sentence like 'configuring pad button 0'", "pad button %1", modelData + 1)
                supportsPenButton: false

                Connections {
                    target: tabletEvents
                    function onPadButtonReceived(path, button, pressed) {
                        if (button !== modelData || !path.endsWith(form.padDevice.sysName)) {
                            return;
                        }
                        seq.buttonPressed = pressed
                    }
                }

                function refreshInputSequence(): void {
                    seq.inputSequence = kcm.padButtonMapping(form.padDevice.name, modelData)
                }

                inputSequence: kcm.padButtonMapping(form.padDevice.name, modelData)
                Connections {
                    target: kcm
                    function onSettingsRestored() {
                        refreshInputSequence();
                    }
                }

                onGotInputSequence: sequence => {
                    kcm.assignPadButtonMapping(form.padDevice.name, modelData, sequence)
                }

                SettingHighlighter {
                    // Currently, application-defined is the default
                    highlight: seq.inputSequence.type !== InputSequence.ApplicationDefined
                }
            }
        }
    }

    ActionDialog {
        id: actionDialog

        parent: root.QQC2.Overlay.overlay
    }
}
