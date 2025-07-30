/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm as KCM

QQC2.ApplicationWindow {
    id: root

    required property KCM.InputDevice device
    required property KCM.TabletEvents tabletEvents

    property KCM.CalibrationTool tool: KCM.CalibrationTool {
        onCalibrationCreated: matrix => tool.setCalibrationMatrix(device, matrix)
        onResetFromSaved: root.device.resetCalibrationMatrix()
    }

    property real currentPenToolX: -1
    property real currentPenToolY: -1

    title: i18nc("@title", "Pen Calibration")

    onWidthChanged: tool.width = width
    onHeightChanged: tool.height = height

    onClosing: {
        // Make sure to reset it back to default if the user closes the window for any reason and hasn't confirmed
        if (tool.state === KCM.CalibrationTool.Confirming) {
            root.device.resetCalibrationMatrix()
        }
    }

    HoverHandler {
        cursorShape: Qt.BlankCursor
        acceptedDevices: PointerDevice.Stylus
        enabled: tool.state !== KCM.CalibrationTool.Testing
    }

    Connections {
        target: tabletEvents

        function onToolDown(hardware_serial_hi: int, hardware_serial_lo: int, x: real, y: real): void {
            const centerX = root.width / 2;
            const centerY = root.height / 2;

            let targetItem = null;
            if (x < centerX && y < centerY) {
                targetItem = topLeftAnchor;
            } else if (x < centerX && y > centerY) {
                targetItem = bottomLeftAnchor;
            } else if (x > centerX && y < centerY) {
                targetItem = topRightAnchor;
            } else {
                targetItem = bottomRightAnchor;
            }

            // Calculate the distance between the item and the point
            const distance = Math.sqrt(Math.pow(x - targetItem.x, 2) + Math.pow(y - targetItem.y, 2));

            // Ensure that large distances are culled because they are probably not clicking on the target
            if (distance > 100) {
                return;
            }

            if (tool.state === KCM.CalibrationTool.Testing) {
                root.currentPenToolX = x;
                root.currentPenToolY = y;
            } else {
                tool.calibrate(x, y, targetItem.positionX, targetItem.positionY);
            }
        }

        function onToolUp(hardware_serial_hi: int, hardware_serial_lo: int, x: real, y: real): void {
            root.currentPenToolX = -1;
            root.currentPenToolY = -1;
        }
    }

    ColumnLayout {
        anchors.centerIn: parent

        spacing: Kirigami.Units.largeSpacing

        QQC2.Label {
            horizontalAlignment: Text.AlignHCenter

            text: {
                if (tool.state === KCM.CalibrationTool.Confirming) {
                    return xi18nc("@info", "Tap the targets again to confirm the new calibration.<nl/><nl/><b>Will revert to default calibration in %1 seconds unless further action is taken.</b>", tool.resetSecondsLeft)
                }

                if (tool.state === KCM.CalibrationTool.Testing) {
                    return xi18nc("@info", "Calibration is completed and saved.<nl/><nl/>Refine the calibration further or close the window.");
                }

                return i18nc("@info", "Tap the center of each target.");
            }
        }

        QQC2.Button {
            visible: tool.state === KCM.CalibrationTool.Testing

            text: i18nc("@action:button", "Refine Existing Calibration")
            icon.name: "edit-redo"

            onClicked: tool.reset()

            Layout.alignment: Qt.AlignHCenter
        }

        QQC2.Button {
            text: tool.state === KCM.CalibrationTool.Confirming ? i18nc("@action:button", "Reset and Try Again") : i18nc("@action:button", "Calibrate from Scratch")
            icon.name: "kt-restore-defaults"
            focus: true
            visible: tool.state === KCM.CalibrationTool.Confirming || tool.state === KCM.CalibrationTool.Testing

            onClicked: {
                tool.restoreDefaults(root.device);
                tool.reset();
            }

            Layout.alignment: Qt.AlignHCenter
        }

        QQC2.Button {
            text: tool.state === KCM.CalibrationTool.Testing ? i18nc("@action:button", "Close") : i18nc("@action:button", "Cancel")
            icon.name: "dialog-cancel"

            onClicked: root.close()

            Layout.alignment: Qt.AlignHCenter
        }
    }

    component Target: QQC2.ToolButton {
        readonly property real positionX: test.Kirigami.ScenePosition.x
        readonly property real positionY: test.Kirigami.ScenePosition.y

        icon.name: "crosshairs"
        focusPolicy: Qt.NoFocus
        hoverEnabled: false
        HoverHandler {
            cursorShape: Qt.BlankCursor
            acceptedDevices: PointerDevice.Stylus
            enabled: tool.state !== KCM.CalibrationTool.Testing
        }

        down: {
            const isX = root.currentPenToolX >= Kirigami.ScenePosition.x && root.currentPenToolX <= Kirigami.ScenePosition.x + width;
            const isY = root.currentPenToolY >= Kirigami.ScenePosition.y && root.currentPenToolY <= Kirigami.ScenePosition.y + height;
            return isX && isY;
        }

        Item {
            id: test

            anchors.centerIn: parent
        }
    }

    component TargetPath: Shape {
        required property real sourceX
        required property real sourceY

        required property real targetX
        required property real targetY

        Kirigami.Theme.colorSet: Kirigami.Theme.Header
        Kirigami.Theme.inherit: false
        ShapePath {
            strokeWidth: 1
            strokeColor: Qt.lighter(Kirigami.Theme.backgroundColor, 1.5)
            startX: sourceX; startY: sourceY
            PathLine { x: targetX; y: targetY }
        }
    }

    component PairTargetPath: Item {
        required property Item anchor

        visible: anchor.visible

        TargetPath {
            sourceX: anchor.positionX
            sourceY: 0
            targetX: anchor.positionX
            targetY: root.height
        }

        TargetPath {
            sourceX: 0
            sourceY: anchor.positionY
            targetX: root.width
            targetY: anchor.positionY
        }
    }

    Target {
        id: topLeftAnchor

        visible: tool.state === KCM.CalibrationTool.Testing || tool.currentTarget === 0

        anchors {
            left: parent.left
            leftMargin: Kirigami.Units.gridUnit * 4
            top: parent.top
            topMargin: Kirigami.Units.gridUnit * 4
        }
    }

    PairTargetPath {
        anchor: topLeftAnchor
    }

    Target {
        id: topRightAnchor

        visible: tool.state === KCM.CalibrationTool.Testing || tool.currentTarget === 1

        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.gridUnit * 4
            top: parent.top
            topMargin: Kirigami.Units.gridUnit * 4
        }
    }

    PairTargetPath {
        anchor: topRightAnchor
    }

    Target {
        id: bottomLeftAnchor

        visible: tool.state === KCM.CalibrationTool.Testing || tool.currentTarget === 2

        anchors {
            left: parent.left
            leftMargin: Kirigami.Units.gridUnit * 4
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.gridUnit * 4
        }
    }

    PairTargetPath {
        anchor: bottomLeftAnchor
    }

    Target {
        id: bottomRightAnchor

        visible: tool.state === KCM.CalibrationTool.Testing || tool.currentTarget === 3

        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.gridUnit * 4
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.gridUnit * 4
        }
    }

    PairTargetPath {
        anchor: bottomRightAnchor
    }
}
