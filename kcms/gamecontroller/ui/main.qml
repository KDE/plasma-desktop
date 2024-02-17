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

    readonly property var device: deviceModel.device(deviceCombo.currentIndex)

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

                textRole: "display"

                Layout.fillWidth: true
            }
        }

        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Layout.alignment: Qt.AlignTop

                QQC2.Label {
                    text: i18nc("@label Visual representation of an axis position", "Position:")
                    textFormat: Text.PlainText
                }

                PositionWidget {
                    id: posWidget

                    device: root.device
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
                    model: AxesModel {
                        device: root.device
                    }

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }
}
