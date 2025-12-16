/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2025 Yelsin Sepulveda <yelsin.sepulveda@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as AddonFormCard

import org.kde.plasma.gamecontroller.kcm

KCM.SimpleKCM {
    id: root

    property var selectedDevice: null
    required property var deviceModel
    readonly property var device: selectedDevice
    readonly property var deviceType: device?.type ?? ""
    readonly property var deviceControllerType: device?.controllerTypeName ?? ""
    readonly property var deviceConnectionType: device?.connectionType ?? ""

    Kirigami.PlaceholderMessage {
        icon.name: "input-gamepad"
        text: i18n("No game controllers found")
        explanation: i18n("Connect a wired or wireless controller")
        anchors.centerIn: parent
        visible: deviceModel.count === 0
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }

    Connections {
        target: deviceModel
        function onDevicesChanged() {
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

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        visible: deviceCombo.count !== 0

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

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

                onCurrentValueChanged: {
                    root.selectedDevice = currentValue !== null ? deviceModel.device(currentValue) : null
                }

                Component.onCompleted: {
                    root.selectedDevice = currentValue !== null ? deviceModel.device(currentValue) : null
                }
            }

        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Item { Layout.fillHeight: true }
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

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

                QQC2.Switch {
                    text: i18nc("@label:textbox", "Enable")
                    checked: KWinPlugin.pluginEnabled
                    anchors.leftMargin: Kirigami.Units.largeSpacing * 2

                    onToggled: {
                        KWinPlugin.pluginEnabled = checked
                    }
                }

                Kirigami.ContextualHelpButton {
                    toolTipText: i18nc("@label:textbox", "Apps may still use the controller while this option is disabled, but Plasma Desktop itself will not detect or use it for desktop navigation or preventing system from entering suspend mode.")
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
