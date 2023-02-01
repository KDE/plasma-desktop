/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.13 as Kirigami
import org.kde.kcm 1.4 as KCM
import QtQuick.Controls 2.0 as QQC2
import QtQuick.Layouts 1.3 as Layouts
import org.kde.plasma.gamepad.kcm 1.0

KCM.SimpleKCM {
    id: root
    KCM.ConfigModule.quickHelp: i18n("This module lets you test and configure game controllers.")

    DeviceModel {
        id: deviceModel
    }

    property var joystickCount: deviceSelector.count

    Kirigami.PlaceholderMessage {
        text: i18n("No gamepad found")
        anchors.centerIn: parent
        visible: joystickCount === 0
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }

    Kirigami.FormLayout {
        id: formLayout

        anchors.fill: parent

        visible: joystickCount > 0

        // Device
        QQC2.ComboBox {
            Kirigami.FormData.label: i18nd("kcm_joystick", "Device:")
            id: deviceSelector

            Layouts.Layout.fillWidth: true
            model: deviceModel
            textRole: "name"
        }

        ListView {
            model: deviceModel

            delegate: QQC2.ItemDelegate {
                required property var name
                required property var device

                ColumnLayout {
					QQC2.Label {
						text: name + " has " + device.numButtons + " buttons"
					}

					QQC2.Label {
						text: name + " has " + device.numAxes + " axes"
					}

					QQC2.Label {
						text: device.hasRumble ? "Does have rumble" : "Does not have rumble"
					}

					Repeater {
						model: device.buttonState

						QQC2.Label {
							RowLayout {
								QQC2.Label {
									text: "Button " + (index + 1) + ": " + modelData
								}
							}
						}
					}

					Repeater {
						model: device.axisState

						QQC2.Label {
							RowLayout {
								QQC2.Label {
									text: "Axis " + (index + 1) + ": " + modelData
								}
							}
						}
					}
				}
            }
        }
    }
}
