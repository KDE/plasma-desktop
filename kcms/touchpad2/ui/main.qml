/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kcm 1.3
import org.kde.kcmutils 1.0
import org.kde.kquickcontrols 2.0

SimpleKCM {
    id: root

    ConfigModule.buttons: ConfigModule.Default | ConfigModule.Apply

    implicitWidth: Kirigami.Units.gridUnit * 38
    implicitHeight: Kirigami.Units.gridUnit * 35

    property alias touchpad: cb.currentValue

    ColumnLayout {

        anchors.fill: parent

        QQC2.ComboBox {
            id: cb

            model: kcm.touchpadModel

            textRole: "display"
            valueRole: "device"

            onCurrentValueChanged: console.warn("changed", currentValue)
        }

        Kirigami.FormLayout {

            Layout.fillWidth: true
            Layout.fillHeight: true

            QQC2.Label {
                Kirigami.FormData.label: "Name:"
                text: root.touchpad.name
            }
        }
    }
}
