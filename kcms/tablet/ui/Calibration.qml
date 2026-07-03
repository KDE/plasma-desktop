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
    property var recordedResiduals: ({})
    property string testingActionSummary: ""

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

    function targetCenter(target: Item): point {
        return target.mapToItem(root.contentItem, target.width / 2, target.height / 2);
    }

    function resetTestingFeedback(): void {
        root.recordedResiduals = ({});
        root.testingActionSummary = "";
    }

    function testingTargets(): var {
        return [topLeftAnchor, topRightAnchor, bottomLeftAnchor, bottomRightAnchor, centerAnchor];
    }

    function residualSummary(): string {
        const order = ["topLeft", "topRight", "bottomLeft", "bottomRight", "center"];
        const parts = [];

        for (const key of order) {
            const residual = root.recordedResiduals[key];
            if (!residual) {
                continue;
            }

            parts.push(i18nc("@info", "%1: (%2 px, %3 px)", residual.label, Math.round(residual.x), Math.round(residual.y)));
        }

        return parts.join("\n");
    }

    Connections {
        target: tabletEvents

        function onToolDown(hardware_serial_hi: int, hardware_serial_lo: int, x: real, y: real): void {
            if (tool.state === KCM.CalibrationTool.Testing) {
                root.currentPenToolX = x;
                root.currentPenToolY = y;

                let closestTarget = null;
                let closestDistance = Infinity;

                for (const target of root.testingTargets()) {
                    const targetPoint = root.targetCenter(target);
                    const targetDistance = Math.sqrt(Math.pow(x - targetPoint.x, 2) + Math.pow(y - targetPoint.y, 2));
                    if (targetDistance < closestDistance) {
                        closestDistance = targetDistance;
                        closestTarget = target;
                    }
                }

                if (closestTarget && closestDistance <= 100) {
                    const targetPoint = root.targetCenter(closestTarget);
                    const residualX = x - targetPoint.x;
                    const residualY = y - targetPoint.y;

                    if (closestTarget === centerAnchor) {
                        tool.applyCenterCorrection(device, residualX, residualY);
                        root.recordedResiduals = ({});
                        root.testingActionSummary = i18nc("@info", "Applied center correction (%1 px, %2 px). Retap the targets to redo the correction.", Math.round(residualX), Math.round(residualY));
                        return;
                    }

                    const updatedResiduals = Object.assign({}, root.recordedResiduals);
                    updatedResiduals[closestTarget.diagnosticKey] = {
                        label: closestTarget.diagnosticLabel,
                        x: residualX,
                        y: residualY,
                    };
                    root.recordedResiduals = updatedResiduals;
                }

                return;
            }

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

            const targetCenterPoint = root.targetCenter(targetItem);

            // Calculate the distance between the target center and the point.
            const distance = Math.sqrt(Math.pow(x - targetCenterPoint.x, 2) + Math.pow(y - targetCenterPoint.y, 2));

            // Ensure that large distances are culled because they are probably not clicking on the target
            if (distance > 100) {
                return;
            }

            tool.calibrate(x, y, targetCenterPoint.x, targetCenterPoint.y);
        }

        function onToolUp(hardware_serial_hi: int, hardware_serial_lo: int, x: real, y: real): void {
            root.currentPenToolX = -1;
            root.currentPenToolY = -1;
        }
    }

    ColumnLayout {
        anchors {
            top: parent.top
            topMargin: Kirigami.Units.gridUnit * 7
            horizontalCenter: parent.horizontalCenter
        }

        spacing: Kirigami.Units.largeSpacing

        QQC2.Label {
            Layout.maximumWidth: root.width - (Kirigami.Units.gridUnit * 8)
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap

            text: {
                if (tool.state === KCM.CalibrationTool.Confirming) {
                    return xi18nc("@info", "Tap the targets again to confirm the new calibration.<nl/><nl/><emphasis strong='true'>Will revert to default calibration in %1 seconds unless further action is taken.</emphasis>", tool.resetSecondsLeft)
                }

                if (tool.state === KCM.CalibrationTool.Testing) {
                    return i18nc("@info", "Calibration is completed and saved.\n\nTap the center target once to fine-tune alignment. You can retap the other targets to verify the result.");
                }

                return i18nc("@info", "Tap the center of each target.");
            }
        }

        QQC2.Label {
            visible: tool.state === KCM.CalibrationTool.Testing && (root.testingActionSummary.length > 0 || root.residualSummary().length > 0)
            Layout.maximumWidth: root.width - (Kirigami.Units.gridUnit * 8)
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            textFormat: Text.PlainText

            text: {
                const sections = [];

                if (root.testingActionSummary.length > 0) {
                    sections.push(root.testingActionSummary);
                }

                const residualSummary = root.residualSummary();
                if (residualSummary.length > 0) {
                    sections.push(i18nc("@info", "Offset Correction:\n%1", residualSummary));
                }

                return sections.join("\n\n");
            }
        }

        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Layout.alignment: Qt.AlignHCenter

            QQC2.Button {
                visible: tool.state === KCM.CalibrationTool.Testing

                text: i18nc("@action:button", "Refine Existing Calibration")
                icon.name: "edit-redo"

                onClicked: {
                    root.resetTestingFeedback();
                    tool.reset();
                }

                Layout.alignment: Qt.AlignHCenter
            }

            QQC2.Button {
                text: tool.state === KCM.CalibrationTool.Confirming ? i18nc("@action:button", "Reset and Try Again") : i18nc("@action:button", "Calibrate from Scratch")
                icon.name: "kt-restore-defaults"
                focus: true
                visible: tool.state === KCM.CalibrationTool.Confirming || tool.state === KCM.CalibrationTool.Testing

                onClicked: {
                    root.resetTestingFeedback();
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
    }

    component Target: QQC2.ToolButton {
        property string diagnosticKey: ""
        property string diagnosticLabel: ""
        readonly property real positionX: Kirigami.ScenePosition.x + width / 2
        readonly property real positionY: Kirigami.ScenePosition.y + height / 2

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
        diagnosticKey: "topLeft"
        diagnosticLabel: i18nc("@info Name used in the diagnostic log", "Top-left")

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
        diagnosticKey: "topRight"
        diagnosticLabel: i18nc("@info Name used in the diagnostic log", "Top-right")

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
        diagnosticKey: "bottomLeft"
        diagnosticLabel: i18nc("@info Name used in the diagnostic log", "Bottom-left")

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
        diagnosticKey: "bottomRight"
        diagnosticLabel: i18nc("@info Name used in the diagnostic log", "Bottom-right")

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

    Target {
        id: centerAnchor
        diagnosticKey: "center"
        diagnosticLabel: i18nc("@info Name used in the diagnostic log", "Center")

        visible: tool.state === KCM.CalibrationTool.Testing

        anchors.centerIn: parent
    }
}
