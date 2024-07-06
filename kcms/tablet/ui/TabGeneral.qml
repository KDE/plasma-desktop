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

    function reloadOutputView() {
        const initialOutputArea = root.device.outputArea;
        if (initialOutputArea === Qt.rect(0, 0, 1, 1)) {
            outputAreaCombo.currentIndex = 0;
        } else if (initialOutputArea.x === 0 && initialOutputArea.y === 0 && initialOutputArea.width === 1) {
            outputAreaCombo.currentIndex = 1;
        } else {
            outputAreaCombo.currentIndex = 2;
        }
        keepAspectRatio.checked = tabletItem.aspectRatio === (root.device.size.width / root.device.size.height)
        outputAreaView.resetOutputArea(outputAreaCombo.currentIndex, initialOutputArea)
    }

    function resetOutputView() {
        outputAreaView.changed = false
    }

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
            enabled: root.device && root.device.supportsLeftHanded
            checked: root.device && root.device.leftHanded
            onCheckedChanged: root.device.leftHanded = checked
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
            if (root.device && changed) {
                root.device.outputArea = outputAreaSetting
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
            readonly property real aspectRatio: outputAreaCombo.currentIndex === 0 ? outputItem.aspectRatio : root.device.size.width / root.device.size.height
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
            outputAreaView.resetOutputArea(outputAreaCombo.currentIndex, root.device.outputArea)
        }
    }
    QQC2.Label {
        text: i18ndc("kcm_tablet", "tablet area position - size", "%1,%2 - %3×%4", String(Math.floor(outputAreaView.outputAreaSetting.x * outputItem.outputSize.width))
            , String(Math.floor(outputAreaView.outputAreaSetting.y * outputItem.outputSize.height))
            , String(Math.floor(outputAreaView.outputAreaSetting.width * outputItem.outputSize.width))
            , String(Math.floor(outputAreaView.outputAreaSetting.height * outputItem.outputSize.height)))
        textFormat: Text.PlainText
    }
}