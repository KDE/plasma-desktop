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
import org.kde.plasma.tablet.kcm

QQC2.ApplicationWindow {
    id: root

    required property var device
    required property var tabletEvents

    property CalibrationTool tool: CalibrationTool {
        onFinished: matrix => {
            tool.setCalibrationMatrix(device, matrix);
        }
    }

    property real currentPenToolX: -1
    property real currentPenToolY: -1

    title: i18nc("@title", "Pen Calibration")

    onWidthChanged: tool.width = width
    onHeightChanged: tool.height = height

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

            if (tool.finishedCalibration) {
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

            text: tool.finishedCalibration
                ? xi18nc("@info", "Calibration is completed and saved.<nl/><nl/>Refine the calibration further or close the window.")
                : xi18nc("@info", "Tap the center of each target.<nl/><nl/>Aim for the point where the stylus tip lands, and ignore the cursor position on the screen.")
        }

        QQC2.Button {
            visible: tool.finishedCalibration

            text: i18nc("@action:button", "Refine Existing Calibration")
            icon.name: "edit-redo"

            onClicked: tool.reset()

            Layout.alignment: Qt.AlignHCenter
        }

        QQC2.Button {
            text: i18nc("@action:button", "Calibrate from Scratch")
            icon.name: "kt-restore-defaults"
            focus: true
            visible: tool.finishedCalibration

            onClicked: {
                tool.restoreDefaults(root.device);
                tool.reset();
            }

            Layout.alignment: Qt.AlignHCenter
        }

        QQC2.Button {
            text: tool.finishedCalibration ? i18nc("@action:button", "Close") : i18nc("@action:button", "Cancel")
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
        required property var anchor

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

        visible: tool.finishedCalibration || tool.currentTarget === 0

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

        visible: tool.finishedCalibration || tool.currentTarget === 1

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

        visible: tool.finishedCalibration || tool.currentTarget === 2

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

        visible: tool.finishedCalibration || tool.currentTarget === 3

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