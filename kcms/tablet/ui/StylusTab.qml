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

Item {
    id: root

    required property var db
    required property var device
    required property var tabletEvents

    readonly property bool calibrationWindowOpen: calibrationWindow !== null
    property Calibration calibrationWindow
    property string currentCalibrationSysName

    signal settingsRestored()

    Repeater {
        id: buttonLineConnectionRepeater

        model: buttonRepeater.count
        delegate: Shape {
            id: delegate

            required property int index

            readonly property Item item: buttonRepeater.itemAt(index)
            readonly property Item button: {
                switch (index) {
                    default:
                    case 0:
                        return firstButton;
                    case 1:
                        return secondButton;
                    case 2:
                        return thirdButton;
                }
            }

            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer

            ShapePath {
                id: path

                fillColor: "transparent"
                strokeWidth: 1
                strokeColor: Qt.alpha(Kirigami.Theme.textColor, 0.5)

                readonly property real beginX: form.Kirigami.ScenePosition.x - root.Kirigami.ScenePosition.x;
                readonly property real endX: delegate.button.Kirigami.ScenePosition.x - root.Kirigami.ScenePosition.x + delegate.button.width
                readonly property real midX: (beginX + endX) / 2.0 - (Kirigami.Units.gridUnit * delegate.index)

                readonly property real beginY: item.Kirigami.ScenePosition.y - root.Kirigami.ScenePosition.y + (item.height / 2)
                readonly property real endY: delegate.button.Kirigami.ScenePosition.y - root.Kirigami.ScenePosition.y + (delegate.button.height / 2)

                function reloadPosition(): void {
                    startX = beginX
                    startY = beginY
                }

                PathLine {
                    x: path.midX
                    y: path.beginY
                }

                PathLine {
                    x: path.midX
                    y: path.endY
                }

                PathLine {
                    x: path.endX
                    y: path.endY
                }
            }

            Connections {
                target: root

                function onWidthChanged(): void {
                    path.reloadPosition();
                }

                function onHeightChanged(): void {
                    path.reloadPosition();
                }
            }

            // to fixup the layout
            Timer {
                interval: 100
                onTriggered: path.reloadPosition()
            }
        }
    }

    RowLayout {
        anchors.fill: parent

        spacing: 0

        Item {
            Layout.fillWidth: true
        }

        Item {
            id: svgContainer

            Layout.preferredWidth: svgOverlay.implicitWidth + Kirigami.Units.gridUnit * 8
            Layout.fillHeight: true

            Shape {
                id: svgOverlay

                preferredRendererType: Shape.CurveRenderer

                anchors.centerIn: parent

                ShapePath {
                    fillColor: "transparent"
                    strokeColor: Kirigami.Theme.disabledTextColor
                    strokeWidth: 1

                    // Copied from images/pen.svg
                    PathSvg {
                        path: "m15.938 0.21032c-9.2112 0.31493-10.544 2.0519-11.134 5.4178-4.5935 26.212-4.5966 254.2-4.5966 254.2l9.5589 27.513s1.0844 1.8566 6.1713 1.84c0.02443-7e-5 0.04072-0.0142 0.06515-0.0149 0.02443 9e-5 0.04072 0.0142 0.06515 0.0149 5.0869 0.0164 6.1713-1.84 6.1713-1.84l9.5589-27.513s-0.0033-227.98-4.5967-254.2c-0.5896-3.3659-1.9224-5.1029-11.134-5.4178-0.02443-7.2526e-4 -0.04072 6.9535e-4 -0.06515 0-0.02443 6.9535e-4 -0.04072-7.2526e-4 -0.06515 0z"
                    }

                    PathSvg {
                        path: "m13.169 289.18 0.7313 10.032 2.1791 0.5257 2.1632-0.5257 0.7313-10.032h-2.8948z"
                    }
                }
            }

            Rectangle {
                id: firstButton

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                    bottomMargin: 115
                }

                visible: buttonRepeater.count >= 1
                implicitWidth: 10
                implicitHeight: 25

                bottomLeftRadius: Kirigami.Units.cornerRadius
                bottomRightRadius: Kirigami.Units.cornerRadius
                color: "transparent"

                border {
                    color: Kirigami.Theme.textColor
                    width: 1
                }
            }

            Rectangle {
                id: secondButton

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                    bottomMargin: 140
                }

                visible: buttonRepeater.count >= 2
                implicitWidth: 10
                implicitHeight: 35

                topLeftRadius: Kirigami.Units.cornerRadius
                topRightRadius: Kirigami.Units.cornerRadius
                color: "transparent"

                border {
                    color: Kirigami.Theme.textColor
                    width: 1
                }
            }

            Rectangle {
                id: thirdButton

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                    bottomMargin: 185
                }

                visible: buttonRepeater.count >= 3
                implicitWidth: 10
                implicitHeight: 20

                radius: Kirigami.Units.cornerRadius
                color: "transparent"

                border {
                    color: Kirigami.Theme.textColor
                    width: 1
                }
            }
        }

        Kirigami.FormLayout {
            id: form

            Layout.fillWidth: false
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
                id: buttonRepeater

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

                        const window = component.createObject(root, {
                            device: root.device,
                            tabletEvents: root.tabletEvents
                        });
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

        Item {
            Layout.fillWidth: true
        }
    }
}
