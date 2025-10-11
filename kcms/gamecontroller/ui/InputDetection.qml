/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2025 Yelsin Sepulveda <yelsin.sepulveda@kdemail.org>

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

    required property var deviceModel
    required property var device
    readonly property var deviceType: device?.type ?? ""

    Kirigami.PlaceholderMessage {
        icon.name: "input-gamepad"
        text: i18n("No game controllers found")
        explanation: i18n("Connect a wired or wireless controller")
        anchors.centerIn: parent
        visible: deviceModel.count === 0
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }

    ColumnLayout {
        anchors.fill: parent

        visible: deviceModel.count !== 0
        spacing: Kirigami.Units.largeSpacing


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
    }
}
