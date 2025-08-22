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

KCM.AbstractKCM {
    id: root

    property int selectedDeviceId: -1

    readonly property var device: deviceModel.device(selectedDeviceId)

    implicitWidth: Kirigami.Units.gridUnit * 40
    implicitHeight: Kirigami.Units.gridUnit * 35

    actions: [
//        Kirigami.Action {
//            id: typesToPopulate
 
//            text: i18nc("@action:button Change type of gamepad preview", "Preview Type")
//            icon.name: "view-preview"
//        },
        Kirigami.Action {
            text: i18nc("@action:button", "Detailed Information")
            icon.name: "input-gamepad-symbolic"
            onTriggered: kcm.push("AdvancedPage.qml", { device: root.device })
        }
    ]

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent

        icon.name: "input-gamepad"
        text: i18n("No game controllers found")
        explanation: i18n("Connect a wired or wireless controller")
        visible: deviceModel.count === 0
        width: parent.width - (Kirigami.Units.gridUnit * 2)
    }

    Component.onCompleted: deviceModel.refresh();

    DeviceModel {
        id: deviceModel

        function refresh(): void {
            if (deviceModel.count === 0) {
                // No selection possible
                root.selectedDeviceId = -1;
            } else if (!deviceModel.device(root.selectedDeviceId)) {
                // Select first device
                // TODO: Don't hardcode role 258 (IDRole)
                root.selectedDeviceId = deviceModel.data(deviceModel.index(0, 0), 258);
            }
        }

        onDevicesChanged: refresh();
    }

    DeviceTypeModel {
        id: deviceTypeModel
    }

    ColumnLayout {
        anchors.fill: parent
        visible: deviceModel.count > 0

        spacing: 0

        QQC2.ScrollView {
            id: navigationScrollView
            Layout.fillWidth: true

            // TODO: uncomment
            visible: deviceModel.count > 1

            RowLayout {
                spacing: 0

                width: Math.max(implicitWidth, navigationScrollView.width)

                // Surrounding items around layout provide centering when
                // excessive free space is available, without taking extra
                // space when not available due to uniformCellSizes.

                Item {
                    Layout.fillWidth: true
                }

                RowLayout {
                    spacing: 0
                    uniformCellSizes: true

                    Repeater {
                        model: deviceModel
                        delegate: Kirigami.NavigationTabButton {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.minimumWidth: Kirigami.Units.gridUnit * 8
                            Layout.maximumWidth: Kirigami.Units.gridUnit * 12
                            Layout.fillWidth: true

                            text: model.text
                            icon.name: "input-gamepad-symbolic"

                            Component.onCompleted: console.log("should be", model.id);

                            checked: root.selectedDeviceId === model.id
                            onClicked: root.selectedDeviceId = model.id
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true

            visible: navigationScrollView.visible
        }

        Rectangle {
            id: controllerContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View

            color: Kirigami.Theme.backgroundColor

            GamepadRoot {
                id: gamepadgui
                anchors.fill: parent

                device: root.device
                svgPath: deviceTypeModel.pathFromRow(deviceTypeCombo.currentIndex)
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            QQC2.ComboBox {
                id: deviceTypeCombo
                Kirigami.FormData.label: i18nc("@label", "Gamepad Type:")

                model: deviceTypeModel

                textRole: "name"
                valueRole: "type"

                onActivated: {
                    gamepadgui.resize();
                }
            }

            Kirigami.SelectableLabel {
                Kirigami.FormData.label: i18nc("@label game controller device type (wheel, joystick, game controller, etc.)", "Device type:")
                text: device?.type ?? ""
            }

            Kirigami.SelectableLabel {
                Kirigami.FormData.label: i18nc("@label game controller controller type (which brand, etc.)", "Controller type:")
                text: device?.controllerTypeName ?? ""
            }

            Kirigami.SelectableLabel {
                Kirigami.FormData.label: i18nc("@label", "Connection Type:")
                text: device?.connectionType ?? ""
            }
        }
    }
}
