/*
    Copyright 2020  Carson Black <uhhadd@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
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