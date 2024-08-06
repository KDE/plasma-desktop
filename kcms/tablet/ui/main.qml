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

SimpleKCM {
    id: root

    property bool testerWindowOpen: false
    property bool calibrationWindowOpen: false

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

    Kirigami.FormLayout {
        id: form
        visible: combo.count > 0
        enabled: combo.count > 0
        QQC2.ComboBox {
            id: combo
            Kirigami.FormData.label: i18ndc("kcm_tablet", "@label:listbox The device we are configuring", "Device:")
            model: kcm.toolsModel

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
                keepAspectRatio.checked = tabletItem.aspectRatio === (form.device.size.width / form.device.size.height)
                outputAreaView.resetOutputArea(outputAreaCombo.currentIndex, initialOutputArea)
            }

            onCurrentIndexChanged: {
                parent.device = kcm.toolsModel.deviceAt(combo.currentIndex)
                reloadOutputView()
            }

            Connections {
                target: kcm
                function onSettingsRestored() {
                    outputAreaView.changed = false
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
            enabled: count > 3 //It's only interesting when there's more than 1 screen
            currentIndex: {

                if (count === 0) {
                    return -1
                }

                outputsModel.rowForDevice(parent.device)
            }
            textRole: "display"
            onActivated: {
                parent.device.outputName = outputsModel.outputNameAt(currentIndex)
                parent.device.mapToWorkspace = outputsModel.isMapToWorkspaceAt(currentIndex)
            }
        }
        QQC2.ComboBox {
            Kirigami.FormData.label: i18nd("kcm_tablet", "Orientation:")
            model: OrientationsModel {
                id: orientationsModel
            }
            enabled: parent.device && parent.device.supportsOrientation
            currentIndex: orientationsModel.rowForOrientation(parent.device.orientation)
            displayText: parent.device.supportsOrientation ? currentText : i18nd("kcm_tablet", "Not Supported")
            textRole: "display"
            onActivated: {
                parent.device.orientation = orientationsModel.orientationAt(currentIndex)
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
        }
        QQC2.ComboBox {
            id: outputAreaCombo
            Layout.fillWidth: true
            Kirigami.FormData.label: i18nd("kcm_tablet", "Mapped Area:")
            model: OutputsFittingModel {}
            onActivated: index => {
                outputAreaView.changed = true
                keepAspectRatio.checked = true
                outputAreaView.resetOutputArea(index, index === 0 ? Qt.rect(0,0, 1,1) : Qt.rect(0, 0, 1, outputItem.aspectRatio/tabletItem.aspectRatio))
            }
        }

        // Display fit demo
        Item {
            id: outputAreaView
            function resetOutputArea(mode, outputArea) {
                if (mode === 0) {
                    tabletItem.x = 0
                    tabletItem.y = 0
                    tabletItem.width   = Qt.binding(() => outputItem.width);
                    tabletItem.height  = Qt.binding(() => outputItem.height);
                } else {
                    tabletItem.x       = Qt.binding(() => outputArea.x * outputItem.width)
                    tabletItem.y       = Qt.binding(() => outputArea.y * outputItem.height)
                    tabletItem.width   = Qt.binding(() => tabletSizeHandle.x);
                    tabletItem.height  = Qt.binding(() => tabletSizeHandle.y);
                    tabletSizeHandle.x = Qt.binding(() => outputArea.width * outputItem.width)
                    if (keepAspectRatio.checked) {
                        tabletSizeHandle.y = Qt.binding(() => tabletSizeHandle.x / tabletItem.aspectRatio);
                    } else {
                        tabletSizeHandle.y = Qt.binding(() => outputArea.height * outputItem.height)
                    }

                }
            }

            readonly property rect outputAreaSetting: Qt.rect(tabletItem.x/outputItem.width, tabletItem.y/outputItem.height,
                                                           tabletItem.width/outputItem.width, tabletItem.height/outputItem.height)
            property bool changed: false
            onOutputAreaSettingChanged: {
                if (form.device && changed) {
                    form.device.outputArea = outputAreaSetting
                }
            }

            Layout.fillWidth: true
            Layout.preferredHeight: Math.max(outputItem.height, tabletItem.height)
            enabled: parent.device

            Output {
                id: outputItem
                readonly property size outputSize: outputsModel.data(outputsModel.index(outputsCombo.currentIndex, 0), Qt.UserRole + 2)
                readonly property real aspectRatio: outputSize.width / outputSize.height
                width: parent.width * 0.7
                height: width / aspectRatio
            }

            Rectangle {
                id: tabletItem
                color: Kirigami.Theme.activeBackgroundColor
                opacity: 0.8
                readonly property real aspectRatio: outputAreaCombo.currentIndex === 0 ? outputItem.aspectRatio : form.device.size.width / form.device.size.height
                width: tabletSizeHandle.x
                height: tabletSizeHandle.y

                DragHandler {
                    cursorShape: Qt.ClosedHandCursor
                    target: parent
                    enabled: outputAreaCombo.currentIndex >= 2
                    onActiveChanged: { outputAreaView.changed = true }

                    xAxis.minimum: 0
                    xAxis.maximum: outputItem.width - tabletItem.width

                    yAxis.minimum: 0
                    yAxis.maximum: outputItem.height - tabletItem.height

                }

                TapHandler {
                    gesturePolicy: TapHandler.WithinBounds
                }

                QQC2.Button {
                    id: tabletSizeHandle
                    x: outputItem.width
                    y: outputItem.width / parent.aspectRatio
                    visible: outputAreaCombo.currentIndex >= 2
                    icon.name: "transform-move"
                    display: QQC2.AbstractButton.IconOnly
                    text: i18nd("kcm_tablet", "Resize the tablet area")
                    QQC2.ToolTip {
                        text: tabletSizeHandle.text
                        visible: parent.hovered
                        delay: Kirigami.Units.toolTipDelay
                    }

                    DragHandler {
                        cursorShape: Qt.SizeFDiagCursor
                        target: parent
                        onActiveChanged: { outputAreaView.changed = true }

                        xAxis.minimum: 10
                        xAxis.maximum: outputItem.width - tabletItem.x

                        yAxis.minimum: keepAspectRatio.checked ? (tabletItem.width / tabletItem.aspectRatio) : 10
                        yAxis.maximum: keepAspectRatio.checked ? (tabletItem.width / tabletItem.aspectRatio) : outputItem.height - tabletItem.y
                    }
                }
            }
        }

        QQC2.CheckBox {
            id: keepAspectRatio
            text: i18ndc("kcm_tablet", "@option:check", "Lock aspect ratio")
            visible: outputAreaCombo.currentIndex >= 2
            checked: true
            onToggled: {
                outputAreaView.resetOutputArea(outputAreaCombo.currentIndex, form.device.outputArea)
            }
        }
        QQC2.Label {
            text: i18ndc("kcm_tablet", "tablet area position - size", "%1,%2 - %3×%4", String(Math.floor(outputAreaView.outputAreaSetting.x * outputItem.outputSize.width))
                                                                                    , String(Math.floor(outputAreaView.outputAreaSetting.y * outputItem.outputSize.height))
                                                                                    , String(Math.floor(outputAreaView.outputAreaSetting.width * outputItem.outputSize.width))
                                                                                    , String(Math.floor(outputAreaView.outputAreaSetting.height * outputItem.outputSize.height)))
            textFormat: Text.PlainText
        }

        Repeater {
            model: [
                {
                    value: 0x14b, // BTN_STYLUS
                    text: i18nd("kcm_tablet", "Pen button 1:"),
                    name: i18ndc("kcm_tablet", "@info Meant to be inserted into an existing sentence like 'configuring pen button 1'", "pen button 1")
                },
                {
                    value: 0x14c, // BTN_STYLUS2
                    text: i18nd("kcm_tablet", "Pen button 2:"),
                    name: i18ndc("kcm_tablet", "@info Meant to be inserted into an existing sentence like 'configuring pen button 2'", "pen button 2")
                },
                {
                    value: 0x149, // BTN_STYLUS3
                    text: i18nd("kcm_tablet", "Pen button 3:"),
                    name: i18ndc("kcm_tablet", "@info Meant to be inserted into an existing sentence like 'configuring pen button 3'", "pen button 3")
                }
            ]

            delegate: ActionBinding {
                id: seq

                required property var modelData

                Kirigami.FormData.label: (buttonPressed ? "<b>" : "") + modelData.text + (buttonPressed ? "</b>" : "")
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

                    const window = component.createObject(root, {device: form.device, tabletEvents, screen: Qt.application.screens[screenIndex]});
                    window.showFullScreen();
                    window.closing.connect((close) => {
                        root.calibrationWindowOpen = false;
                    });

                    root.calibrationWindowOpen = true;
                }
            }

            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.text: i18nc("@info:tooltip", "Calibration across multiple screens is not supported.")
            QQC2.ToolTip.visible: hovered && outputsModel.isMapToWorkspaceAt(outputsCombo.currentIndex)
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        property QtObject padDevice: null
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

        Repeater {
            id: buttonsRepeater
            model: tabletEvents.padButtons

            delegate: ActionBinding {
                id: seq

                required property var modelData

                Kirigami.FormData.label: (buttonPressed ? "<b>" : "") + i18nd("kcm_tablet", "Pad button %1:", modelData + 1) + (buttonPressed ? "</b>" : "")
                property bool buttonPressed: false

                name: i18ndc("kcm_tablet", "@info Meant to be inserted into an existing sentence like 'configuring pad button 0'", "pad button %1", modelData + 1)

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
            }
        }
    }

    ActionDialog {
        id: actionDialog
    }
}
