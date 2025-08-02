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

Item {
    id: root

    required property var db
    required property KCM.InputDevice device
    required property KCM.TabletEvents tabletEvents

    readonly property bool calibrationWindowOpen: calibrationWindow !== null
    property Calibration calibrationWindow
    property string currentCalibrationSysName
    // TODO: don't allow calibration across multiple screens again
    readonly property bool supportsCalibration: root.device.supportsCalibrationMatrix

    signal settingsRestored

    implicitHeight: layout.implicitHeight + Kirigami.Units.largeSpacing * 2

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

                readonly property real beginX: form.Kirigami.ScenePosition.x - root.Kirigami.ScenePosition.x
                readonly property real endX: delegate.button.Kirigami.ScenePosition.x - root.Kirigami.ScenePosition.x + delegate.button.width
                readonly property real midX: (beginX + endX) / 2.0 + (Kirigami.Units.gridUnit * delegate.index)

                readonly property real beginY: item.Kirigami.ScenePosition.y - root.Kirigami.ScenePosition.y + (item.height / 2)
                readonly property real endY: delegate.button.Kirigami.ScenePosition.y - root.Kirigami.ScenePosition.y + (delegate.button.height / 2)

                function reloadPosition(): void {
                    startX = Qt.binding(() => beginX);
                    startY = Qt.binding(() => beginY);
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
        }
    }

    RowLayout {
        id: layout

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
                        path: "m15.938 299.77c-9.2112-0.315-10.544-2.052-11.134-5.418-4.5935-26.212-4.5966-254.2-4.5966-254.2l9.5589-27.513s1.0844-1.8566 6.1713-1.84c0.0244 1e-4 0.0407 0.0142 0.0651 0.0149 0.0245-1e-4 0.0408-0.0142 0.0652-0.0149 5.0869-0.0164 6.1713 1.84 6.1713 1.84l9.5589 27.513s-0.0033 227.98-4.5967 254.2c-0.5896 3.366-1.9224 5.103-11.134 5.418-0.0244 0-0.0407-1e-3 -0.0651 0-0.0245-1e-3 -0.0408 0-0.0652 0z"
                    }

                    PathSvg {
                        path: "m13.169 10.8 0.7313-10.032 2.1791-0.5257 2.1632 0.5257 0.7313 10.032h-2.8948z"
                    }
                }

                Rectangle {
                    id: firstButton

                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        top: parent.top
                        topMargin: 55
                    }

                    visible: buttonRepeater.count >= 1
                    implicitWidth: 10
                    implicitHeight: 25

                    topLeftRadius: Kirigami.Units.cornerRadius
                    topRightRadius: Kirigami.Units.cornerRadius
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
                        top: parent.top
                        topMargin: 79
                    }

                    visible: buttonRepeater.count >= 2
                    implicitWidth: 10
                    implicitHeight: 35

                    bottomLeftRadius: Kirigami.Units.cornerRadius
                    bottomRightRadius: Kirigami.Units.cornerRadius
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
                        top: parent.top
                        topMargin: 125
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

            // Click behavior settings
            QQC2.ButtonGroup { id: isRelativeGroup }

            ColumnLayout {
                Kirigami.FormData.label: i18nc("'Mode' is the mode the stylus is in, pen (absolute) or mouse (relative)", "Mode:")
                Kirigami.FormData.buddyFor: absoluteMode

                Layout.fillWidth: true
                spacing: 0

                QQC2.RadioButton {
                    id: absoluteMode
                    text: i18nc("Pen mode, where the point on the screen is always where you touch the stylus", "Pen")
                    checked: root.device && !root.device.relative
                    onToggled: root.device.relative = false
                    QQC2.ButtonGroup.group: isRelativeGroup

                    Accessible.description: i18n("The cursor follows where you touch the pen on the surface")
                }

                QQC2.Label {
                    // Layout.fillWidth: true
                    leftPadding: absoluteMode.indicator.width
                    text: absoluteMode.Accessible.description
                    textFormat: Text.PlainText
                    elide: Text.ElideRight
                    font: Kirigami.Theme.smallFont
                    Layout.maximumWidth: 300
                    wrapMode: Text.Wrap
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                QQC2.RadioButton {
                    id: relativeMode
                    text: i18nc("Mouse mode, or like using the tablet like a giant touchpad", "Mouse")
                    checked: root.device && root.device.relative
                    onToggled: root.device.relative = true
                    QQC2.ButtonGroup.group: isRelativeGroup

                    Accessible.description: i18n("Moving the pen on the surface moves the cursor relative to where it was, like a mouse")
                }

                QQC2.Label {
                    Layout.fillWidth: true
                    leftPadding: relativeMode.indicator.width
                    text: relativeMode.Accessible.description
                    textFormat: Text.PlainText
                    elide: Text.ElideRight
                    font: Kirigami.Theme.smallFont
                    Layout.maximumWidth: 300
                    wrapMode: Text.Wrap
                }
            }

            Repeater {
                id: buttonRepeater

                model: KCM.StylusButtonsModel {
                    device: root.device
                    db: root.db
                }

                delegate: ActionBinding {
                    id: seq

                    required property string label
                    required property int index
                    required name

                    Kirigami.FormData.label: (buttonPressed ? "<b>" : "") + label + (buttonPressed ? "</b>" : "")
                    property bool buttonPressed: false

                    Connections {
                        target: root.tabletEvents

                        function onToolButtonReceived(hardware_serial_hi: int, hardware_serial_lo: int, button: int, pressed: bool): void {
                            if (button !== seq.index) {
                                return;
                            }
                            seq.buttonPressed = pressed;
                        }
                    }

                    Connections {
                        target: root

                        function onSettingsRestored(): void {
                            refreshInputSequence();
                        }
                    }

                    function refreshInputSequence(): void {
                        seq.inputSequence = kcm.toolButtonMapping(root.device.name, seq.index);
                    }

                    Component.onCompleted: refreshInputSequence()

                    onGotInputSequence: sequence => {
                        kcm.assignToolButtonMapping(root.device.name, seq.index, sequence);
                    }

                    SettingHighlighter {
                        // Currently, application-defined is the default
                        highlight: seq.inputSequence.type !== KCM.InputSequence.ApplicationDefined
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

                        property bool loadedSettings: false

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

                            loadedSettings = true;
                        }

                        function saveSettings(): void {
                            // We need to make sure not to re-save the settings we are loading in reloadSettings()
                            if (!root.device || !loadedSettings) {
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

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                QQC2.Button {
                    text: {
                        if (root.supportsCalibration) {
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
                    enabled: root.supportsCalibration && !root.calibrationWindowOpen
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
                            window.closing.connect(close => {
                                root.calibrationWindow = null;
                            });

                            root.currentCalibrationSysName = root.device.sysName;
                            root.calibrationWindow = window;
                        }
                    }

                    SettingHighlighter {
                        highlight: !root.device.calibrationMatrixIsDefault
                    }
                }

                QQC2.Button {
                    text: i18nc("@action:button", "Reset Custom Calibration")
                    icon.name: "edit-undo-symbolic"
                    enabled: !root.device.calibrationMatrixIsDefault
                    visible: root.supportsCalibration
                    display: QQC2.AbstractButton.IconOnly

                    onClicked: root.device.resetCalibrationMatrix()

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    QQC2.ToolTip.visible: hovered
                }
            }

            QQC2.Label {
                text: i18nc("@info", "You have manually calibrated this tablet. If it's no longer working correctly, try resetting this first.")
                visible: !root.device.calibrationMatrixIsDefault && root.supportsCalibration
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
                font: Kirigami.Theme.smallFont

                Layout.preferredWidth: 260
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
