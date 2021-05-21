/*
    SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.6
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5 as QQC2

import org.kde.kcm 1.2 as KCM
import org.kde.kirigami 2.4 as Kirigami

KCM.SimpleKCM {
    title: i18n("Create User")

    width: mainColumn.childrenRect.width + (Kirigami.Units.largeSpacing*10)
    height: mainColumn.childrenRect.height + (Kirigami.Units.largeSpacing*10)

    onVisibleChanged: {
        userNameField.text = realNameField.text = verifyField.text = passwordField.text = ""
        usertypeBox.currentIndex = 0
    }

    Kirigami.FormLayout {
        anchors.centerIn: parent
        QQC2.TextField {
            id: realNameField
            Kirigami.FormData.label: i18n("Name:")
            Component.onCompleted: realNameField.forceActiveFocus()
        }
        QQC2.TextField {
            id: userNameField
            Kirigami.FormData.label: i18n("Username:")
            validator: RegExpValidator {
                regExp: /^[a-z_]([a-z0-9_-]{0,31}|[a-z0-9_-]{0,30}\$)$/
            }
        }
        QQC2.ComboBox {
            id: usertypeBox

            textRole: "label"
            model: [
                { "type": "standard", "label": i18n("Standard") },
                { "type": "administrator", "label": i18n("Administrator") },
            ]

            Kirigami.FormData.label: i18n("Account type:")
        }
        QQC2.TextField {
            id: passwordField
            echoMode: TextInput.Password
            Kirigami.FormData.label: i18n("Password:")
        }
        QQC2.TextField {
            id: verifyField
            echoMode: TextInput.Password
            Kirigami.FormData.label: i18n("Confirm password:")
        }
        Kirigami.InlineMessage {
            id: passwordWarning

            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            text: i18n("Passwords must match")
            visible: passwordField.text != "" && verifyField.text != "" && passwordField.text != verifyField.text
        }
        QQC2.Button {
            text: i18n("Create")
            enabled: !passwordWarning.visible && verifyField.text && passwordField.text && realNameField.text && userNameField.text
            onClicked: {
                if (kcm.createUser(userNameField.text, realNameField.text, passwordField.text, (usertypeBox.model[usertypeBox.currentIndex]["type"] == "administrator"))) {
                    kcm.pop()
                }
            }
        }
    }
}