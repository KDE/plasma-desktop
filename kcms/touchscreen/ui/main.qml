/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    Work sponsored by Technische Universität Dresden:
    SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.plasma.touchscreen.kcm 1.0
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property var device: null

    KCM.ConfigModule.buttons: KCM.ConfigModule.Default | KCM.ConfigModule.Apply

    implicitWidth: Kirigami.Units.gridUnit * 38
    implicitHeight: Kirigami.Units.gridUnit * 35

    Kirigami.PlaceholderMessage {
        text: i18n("No touchscreens found")
        anchors.centerIn: parent
        visible: combo.count === 0
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }

    Kirigami.FormLayout {

        visible: combo.count > 0

        QQC2.ComboBox {
            id: combo
            model: kcm.touchscreensModel

            enabled: count > 1 //It's only interesting when there's more than 1 touchscreen

            Kirigami.FormData.label: i18nc("@label:listbox The device we are configuring", "Device:")

            onCountChanged: if (count > 0 && currentIndex < 0) {
                currentIndex = 0;
            }

            onCurrentIndexChanged: {
                root.device = kcm.touchscreensModel.deviceAt(currentIndex)
            }
        }


        QQC2.CheckBox {
            Kirigami.FormData.label: i18n("Enabled:")

            checked: root.device.enabled
            onToggled: root.device.enabled = checked
        }

        QQC2.ComboBox {
            id: outputsCombo
            Kirigami.FormData.label: i18n("Target display:")
            model: OutputsModel {
                id: outputsModel
            }
            enabled: count > 2 //It's only interesting when there's more than 1 screen
            currentIndex: {
                if (count == 0) {
                    return -1;
                }

                return outputsModel.rowForOutputName(root.device.outputName)
            }
            textRole: "display"
            onActivated: {
                root.device.outputName = outputsModel.outputNameAt(currentIndex)
            }
        }
    }
}
