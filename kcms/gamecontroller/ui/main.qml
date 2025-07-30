/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

import org.kde.plasma.gamecontroller.kcm

KCM.SimpleKCM {
    id: root

    readonly property var device: deviceCombo.currentValue !== null ? deviceModel.device(deviceCombo.currentValue) : null
    readonly property var deviceType: device?.type ?? ""
    readonly property var deviceControllerType: device?.controllerTypeName ?? ""
    readonly property var deviceConnectionType: device?.connectionType ?? ""

    implicitWidth: Kirigami.Units.gridUnit * 40
    implicitHeight: Kirigami.Units.gridUnit * 35

    actions: [
//        Kirigami.Action {
//            id: typesToPopulate
 
//            text: i18nc("@action:button Change type of gamepad preview", "Preview Type")
//            icon.name: "view-preview"
//        },
        Kirigami.Action {
            text: i18nc("@action:button", "Controller Information")
            icon.name: "input-gamepad-symbolic"
            onTriggered: kcm.push("AdvancedPage.qml", { device: root.device })
        }
    ]

    Kirigami.PlaceholderMessage {
        icon.name: "input-gamepad"
        text: i18n("No game controllers found")
        explanation: i18n("Connect a wired or wireless controller")
        anchors.centerIn: parent
        visible: deviceModel.count === 0
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }

    DeviceModel {
        id: deviceModel

        onDevicesChanged: {
            // If there are no devices, make sure the combo box is set to no selection
            if (deviceModel.count === 0) {
                deviceCombo.currentIndex = -1;
            } else if (deviceCombo.currentIndex === -1) {
                // However if we didn't have a selection before, and now have a device
                deviceCombo.currentIndex = 0;
            } else if (deviceCombo.currentIndex >= deviceModel.count) {
                // If the last device in the popup list was disconnected, select a previous one
                deviceCombo.currentIndex = deviceModel.count - 1;
            }
        }
    }

    DeviceTypeModel {
        id: deviceTypeModel
    }

    ColumnLayout {
        anchors.fill: parent

        visible: deviceCombo.count !== 0
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Layout.fillWidth: true

            QQC2.Label {
                text: i18nc("@label:textbox", "Device:")
                textFormat: Text.PlainText
            }

            QQC2.ComboBox {
                id: deviceCombo

                model: deviceModel

                textRole: "text"
                valueRole: "id"

                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true

            visible: deviceCombo.count !== 0
            spacing: Kirigami.Units.largeSpacing

            RowLayout {
                spacing: Kirigami.Units.largeSpacing

                Layout.fillWidth: true

                QQC2.Label {
                    text: i18nc("@label:textbox", "Gamepad Type:")
                    textFormat: Text.PlainText
                }

                QQC2.ComboBox {
                    id: deviceTypeCombo

                    model: deviceTypeModel

                    textRole: "name"
                    valueRole: "type"

                    Layout.fillWidth: true

                    onActivated: {
                        gamepadgui.resize();
                    }
                }
            }
        }

        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Layout.fillWidth: true

            QQC2.Label {
                text: i18nc("@label game controller device type (wheel, joystick, game controller, etc.)", "Device type:")
                textFormat: Text.PlainText
            }

            QQC2.Label {
                id: typeLabel
                text: deviceType
            }
        }
        
        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Layout.fillWidth: true

            QQC2.Label {
                text: i18nc("@label game controller controller type (which brand, etc.)", "Controller type:")
                textFormat: Text.PlainText
            }

            QQC2.Label {
                id: controllerTypeLabel
                text: deviceControllerType
            }
        }

        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Layout.fillWidth: true

            QQC2.Label {
                text: i18nc("@label:textbox", "Connection type:")
                textFormat: Text.PlainText
            }

            QQC2.Label {
                id: connectionTypeLabel
                text: deviceConnectionType
            }
        }
        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Layout.fillWidth: true
            Layout.fillHeight: true

            GamepadRoot {
                id: gamepadgui

                Layout.fillWidth: true
                Layout.fillHeight: true

                width: Kirigami.Units.gridUnit * 20
                height: Kirigami.Units.gridUnit * 20

                device: root.device
                svgPath: deviceTypeModel.pathFromRow(deviceTypeCombo.currentIndex)
            }
        }
    }
}
