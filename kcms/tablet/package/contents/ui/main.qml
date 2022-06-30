/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QQC2
import org.kde.kirigami 2.6 as Kirigami
import org.kde.plasma.tablet.kcm 1.0
import org.kde.kcm 1.3

SimpleKCM {
    id: root

    ConfigModule.buttons: ConfigModule.Default | ConfigModule.Apply

    implicitWidth: Kirigami.Units.gridUnit * 38
    implicitHeight: Kirigami.Units.gridUnit * 35

    header: Kirigami.InlineMessage {
        Layout.fillWidth: true

        type: Kirigami.MessageType.Information
        visible: combo.count === 0
        text: i18n("No drawing tablets found.")
    }
    Kirigami.FormLayout {

        enabled: combo.count > 0
        QQC2.ComboBox {
            id: combo
            Kirigami.FormData.label: i18nd("kcmtablet", "Device:")
            model: kcm.devicesModel

            onCurrentIndexChanged: {
                parent.device = kcm.devicesModel.deviceAt(combo.currentIndex)
            }
        }

        property QtObject device: null

        QQC2.ComboBox {
            Kirigami.FormData.label: i18nd("kcmtablet", "Target display:")
            model: OutputsModel {}
            enabled: count > 2 //It's only interesting when there's more than 1 screen
            currentIndex: model.rowForOutputName(parent.device.outputName)
            textRole: "display"
            onActivated: {
                parent.device.outputName = model.outputNameAt(currentIndex)
            }
        }
        QQC2.ComboBox {
            Kirigami.FormData.label: i18nd("kcmtablet", "Orientation:")
            model: OrientationsModel {}
            enabled: parent.device && parent.device.supportsOrientation
            currentIndex: model.rowForOrientation(parent.device.orientation)
            textRole: "display"
            onActivated: {
                parent.device.orientation = model.orientationAt(currentIndex)
            }
        }
        QQC2.CheckBox {
            Kirigami.FormData.label: i18nd("kcmtablet", "Left-handed mode:")
            enabled: parent.device && parent.device.supportsLeftHanded
            checked: parent.device && parent.device.leftHanded
            onCheckedChanged: {
                parent.device.leftHanded = checked
            }
        }
    }
}

