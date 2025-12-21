/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

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

        Item { Layout.fillHeight: true }
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter

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

            QQC2.Switch {
                id: plasmaIntegrationSwitch
                text: i18nc("@label:textbox", "Enable Plasma integration")
                checked: KWinPlugin.pluginEnabled

                onToggled: {
                    KWinPlugin.pluginEnabled = checked
                }
            }

            Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@label:tooltip", "When disabled, this device can't be used to control Plasma or most non-game applications, and using it will not stop the computer from going to sleep or turning off the screen. Games can always use the controller")
            }
        }

        RowLayout {
            spacing: Kirigami.Units.largeSpacing
            Layout.fillWidth: true

            visible: !plasmaIntegrationSwitch.checked
            Layout.preferredHeight: visible ? implicitHeight : 0

            Item {
                implicitWidth: Kirigami.Units.gridUnit * 2
            }

            QQC2.Label {
                text: i18nc("@label:textbox", "Games can always use the controller.")
                textFormat: Text.PlainText
                font: Kirigami.Theme.smallFont
            }
        }
            Item { Layout.fillHeight: true }
    }

        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Layout.alignment: Qt.AlignTop

                QQC2.Label {
                    text: i18nc("@label Visual representation of the axis position for the left axis", "Left position:")
                    textFormat: Text.PlainText
                }

                PositionWidget {
                    id: leftPosWidget

                    device: root.device
                    leftAxis: true
                }
                
                QQC2.Label {
                    text: i18nc("@label Visual representation of the axis position for the right axis", "Right position:")
                    textFormat: Text.PlainText
                }

                PositionWidget {
                    id: rightPosWidget
                    device: root.device
                    leftAxis: false
                }
            }

            ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 50 // Same space for the two columns

                QQC2.Label {
                    text: i18nc("@label Gamepad buttons", "Buttons:")
                    textFormat: Text.PlainText
                }

                Table {
                    model: ButtonModel {
                        device: root.device
                    }

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

            ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 50 // Same space for the two columns

                QQC2.Label {
                    text: i18nc("@label Gamepad axes (sticks)", "Axes:")
                    textFormat: Text.PlainText
                }

                Table {
                    model: AxesProxyModel {
                        device: root.device
                    }

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
        Item { Layout.fillHeight: true }
    }
}

