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

import org.kde.kirigami 2.8 as Kirigami

Kirigami.OverlaySheet {
    id: passwordRoot

    function openAndClear() {
        verifyField.text = ""
        passwordField.text = ""
        passwordField.forceActiveFocus()
        this.open()
    }

    property var account

    header: Kirigami.Heading {
        text: i18n("Change Password")
    }

    ColumnLayout {
        id: mainColumn
        spacing: Kirigami.Units.smallSpacing
        Layout.preferredWidth: Kirigami.Units.gridUnit * 15

        // We don't use a FormLayout here because layouting breaks at small widths.
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            Kirigami.PasswordField {
                id: passwordField
                placeholderText: i18n("Password")

                onAccepted: {
                    if (!passwordWarning.visible && verifyField.text && passwordField.text) {
                        passButton.apply()
                    }
                }

                Layout.alignment: Qt.AlignLeft
            }
            Kirigami.PasswordField {
                id: verifyField
                placeholderText: i18n("Confirm password")

                onAccepted: {
                    if (!passwordWarning.visible && verifyField.text && passwordField.text) {
                        passButton.apply()
                    }
                }

                Layout.alignment: Qt.AlignLeft
            }
            Kirigami.InlineMessage {
                id: passwordWarning

                Layout.fillWidth: true
                Layout.maximumWidth: verifyField.width
                type: Kirigami.MessageType.Error
                text: i18n("Passwords must match")
                visible: passwordField.text != "" && verifyField.text != "" && passwordField.text != verifyField.text
            }
            QQC2.Button {
                id: passButton
                text: i18n("Set Password")
                enabled: !passwordWarning.visible && verifyField.text && passwordField.text
                Layout.alignment: Qt.AlignLeft
                onClicked: apply()
                function apply() {
                    user.password = passwordField.text
                    passwordRoot.close()
                }
            }
        }
    }
}
