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
        text: i18n("No game controllers found")
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
            }

            QQC2.ComboBox {
                id: deviceCombo

                model: deviceModel

                textRole: "name"

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

                QQC2.Label {
                    text: i18nc("@label Gamepad buttons", "Buttons:")
                }

                Table {
                    model: ButtonModel {
                        device: root.device
                    }

                    textRole: "buttonState"

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

            ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Layout.fillWidth: true
                Layout.fillHeight: true

                QQC2.Label {
                    text: i18nc("@label Gamepad axes (sticks)", "Axes:")
                }

                Table {
                    model: AxesModel {
                        device: root.device
                    }

                    textRole: "axisValue"

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }
}
