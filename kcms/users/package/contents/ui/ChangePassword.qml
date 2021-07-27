/*
    SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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

    title: i18n("Change Password")

    ColumnLayout {
        id: mainColumn
        spacing: Kirigami.Units.smallSpacing

        // We don't use a FormLayout here because layouting breaks at small widths.
        ColumnLayout {
            Layout.preferredWidth: Kirigami.Units.gridUnit * 15
            Layout.alignment: Qt.AlignHCenter
            Kirigami.PasswordField {
                id: passwordField

                Layout.fillWidth: true

                placeholderText: i18n("Password")

                onAccepted: {
                    if (!passwordWarning.visible && verifyField.text && passwordField.text) {
                        passButton.apply()
                    }
                }

            }
            Kirigami.PasswordField {
                id: verifyField

                Layout.fillWidth: true

                placeholderText: i18n("Confirm password")

                onAccepted: {
                    if (!passwordWarning.visible && verifyField.text && passwordField.text) {
                        passButton.apply()
                    }
                }
            }
            Kirigami.InlineMessage {
                id: passwordWarning

                Layout.fillWidth: true
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
