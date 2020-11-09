/*
    Copyright 2020  Nate Graham <nate@kde.org>

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
    id: walletPasswordRoot

    header: Kirigami.Heading {
        text: i18n("Change Wallet Password?")
    }

    ColumnLayout {
        id: baseLayout
        Layout.preferredWidth: Kirigami.Units.gridUnit * 27
        spacing: Kirigami.Units.largeSpacing

        QQC2.Label {
            Layout.fillWidth: true
            text: xi18nc("@info", "Now that you have changed your login password, you may also want to change the password on your default KWallet to match it.")
            wrapMode: Text.Wrap
        }

        Kirigami.LinkButton {
            text: i18n("What is KWallet?")
            onClicked: {
                whatIsKWalletExplanation.visible = !whatIsKWalletExplanation.visible
            }
        }

        QQC2.Label {
            id: whatIsKWalletExplanation
            Layout.fillWidth: true
            visible: false
            text: i18n("KWallet is a password manager that stores your passwords for wireless networks and other encrypted resources. It is locked with its own password which differs from your login password. If the two passwords match, it can be unlocked at login automatically so you don't have to enter the KWallet password yourself.")
            wrapMode: Text.Wrap
        }

        // Not using a QQC2.DialogButtonBox because it can only do horizontal layouts
        // and we want the buttons to be stacked when the view is really narrow.
        // TODO: go back to using a DialogButtonBox if this OverlaySheet is ever
        // parented to the KCM as a whole, not this view in particular
        GridLayout {
            readonly property bool narrow: baseLayout.width < Kirigami.Units.gridUnit * 20
            Layout.alignment: narrow ? Qt.AlignHCenter : Qt.AlignRight
            rows: narrow ? 2 : 1
            columns: narrow ? 1 : 2

            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Change Wallet Password")
                icon.name: "lock"
                onClicked: {
                    user.changeWalletPassword()
                    walletPasswordRoot.close()
                }
            }
            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Leave Unchanged")
                icon.name: "dialog-cancel"
                onClicked: {
                    walletPasswordRoot.close()
                }
            }
        }
    }
}
