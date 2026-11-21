/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    Work sponsored by Technische Universität Dresden:
    SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCMUtils
import org.kde.plasma.touchscreen.kcm as TouchScreenKCM

KCMUtils.SimpleKCM {
    id: root

    property TouchScreenKCM.InputDevice device

    KCMUtils.ConfigModule.buttons: KCMUtils.ConfigModule.Default | KCMUtils.ConfigModule.Apply

    implicitWidth: Kirigami.Units.gridUnit * 38
    implicitHeight: Kirigami.Units.gridUnit * 35

    Kirigami.PlaceholderMessage {
        icon.name: "input-touchscreen"
        text: i18nc("@info:status placeholdermessage text", "No touchscreens found")
        explanation: i18nc("@info:usagetip placeholdermessage explanation", "Connect an external touchscreen")
        anchors.centerIn: parent
        visible: combo.count === 0
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }

    Kirigami.Form {
        Kirigami.FormGroup {
            visible: combo.count > 0
            Kirigami.FormEntry {
                title: i18nc("@label:listbox The device we are configuring", "Device:")
                contentItem: QQC2.ComboBox {
                    id: combo
                    model: kcm.touchscreensModel
                    textRole: "display"

                    enabled: count > 1 //It's only interesting when there's more than 1 touchscreen

                    onCountChanged: if (count > 0 && currentIndex < 0) {
                        currentIndex = 0;
                    }

                    onCurrentIndexChanged: {
                        root.device = kcm.touchscreensModel.deviceAt(currentIndex)
                    }
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@title:group currently only entry 'Enable device'", "General:")
                contentItem: QQC2.CheckBox {
                    text: i18nc("@option:check prefix touchscreen device is enabled", "Enable device")

                    checked: root.device?.enabled ?? false
                    onToggled: root.device.enabled = checked
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@label:listbox this is a compound noun", "Target display:")
                contentItem: QQC2.ComboBox {
                    id: outputsCombo
                    model: TouchScreenKCM.OutputsModel {
                        id: outputsModel
                    }
                    enabled: count > 2 //It's only interesting when there's more than 1 screen
                    currentIndex: {
                        if (!root.device || count === 0) {
                            return -1;
                        }

                        return outputsModel.rowForOutputName(root.device.outputName);
                    }
                    textRole: "display"
                    onActivated: {
                        if (root.device) {
                            root.device.outputName = outputsModel.outputNameAt(currentIndex);
                        }
                    }
                }
            }
        }
    }
}
