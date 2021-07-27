/*
    SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.12
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Dialogs 1.3

import org.kde.kcm 1.2
import org.kde.kirigami 2.13 as Kirigami

SimpleKCM {
    id: usersDetailPage

    title: user.realName

    property QtObject user
    property bool overrideImage: false
    property url oldImage

    Connections {
        target: user
        function onApplyError(errorText) {
            errorMessage.visible = true
            errorMessage.text = errorText
        }
    }

    Connections {
        target: user
        function onPasswordSuccessfullyChanged() {
            // Prompt to change the wallet password of the logged-in user
            if (usersDetailPage.user.loggedIn && usersDetailPage.user.usesDefaultWallet()) {
                changeWalletPassword.open()
            }
        }
    }


    Connections {
        target: kcm

        function onApply() {
            errorMessage.visible = false
            usersDetailPage.user.realName = realNametextField.text
            usersDetailPage.user.email = emailTextField.text
            usersDetailPage.user.name = userNameField.text
            usersDetailPage.user.administrator = (usertypeBox.model[usertypeBox.currentIndex]["type"] == "administrator")
            user.apply()
            usersDetailPage.overrideImage = false
            usersDetailPage.oldImage = ""
        }

        function onReset() {
            errorMessage.visible = false
            realNametextField.text = usersDetailPage.user.realName
            emailTextField.text = usersDetailPage.user.email
            userNameField.text = usersDetailPage.user.name
            usertypeBox.currentIndex = usersDetailPage.user.administrator ? 1 : 0
            if (usersDetailPage.oldImage != "") {
                usersDetailPage.overrideImage = false
                usersDetailPage.user.face = usersDetailPage.oldImage
            }
        }
    }

    function resolvePending() {
        let pending = false
        let user = usersDetailPage.user
        pending = pending || user.realName != realNametextField.text
        pending = pending || user.email != emailTextField.text
        pending = pending || user.name != userNameField.text
        pending = pending || user.administrator != (usertypeBox.model[usertypeBox.currentIndex]["type"] == "administrator")
        pending = pending || usersDetailPage.overrideImage
        return pending
    }

    Component.onCompleted: {
        kcm.needsSave = Qt.binding(resolvePending)
    }

    FileDialog {
        id: fileDialog
        title: i18n("Choose a picture")
        folder: shortcuts.pictures
        onAccepted: {
            picturesSheet.close()
            usersDetailPage.oldImage = usersDetailPage.user.face
            usersDetailPage.user.face = fileDialog.fileUrl
            usersDetailPage.overrideImage = true
        }
    }

    ColumnLayout {
        Kirigami.InlineMessage {
            id: errorMessage
            visible: false
            type: Kirigami.MessageType.Error
            Layout.fillWidth: true
        }
        RowLayout {
            Layout.topMargin: Kirigami.Units.gridUnit
            Layout.alignment: Qt.AlignHCenter

            Kirigami.Avatar {
                source: usersDetailPage.user.face
                cache: false
                name: user.realName

                readonly property int size: 6 * Kirigami.Units.gridUnit

                Layout.preferredWidth: size
                Layout.preferredHeight: size

                actions {
                    main: Kirigami.Action {
                        text: i18n("Change avatar")
                        onTriggered: {
                            picturesSheet.open()
                            stackSwitcher.forceActiveFocus()
                        }
                    }
                }
            }
            QQC2.TextField  {
                id: realNametextField
                focus: true
                text: user.realName
                placeholderText: i18nc("Example name", "John Doe")
            }
        }
        Kirigami.FormLayout {
            QQC2.TextField {
                id: userNameField
                focus: true
                text: user.name
                Kirigami.FormData.label: i18n("Username:")
            }

            QQC2.ComboBox {
                id: usertypeBox

                textRole: "label"
                model: [
                    { "type": "standard", "label": i18n("Standard") },
                    { "type": "administrator", "label": i18n("Administrator") },
                ]

                Kirigami.FormData.label: i18n("Account type:")

                currentIndex: user.administrator ? 1 : 0
            }

            QQC2.TextField {
                id: emailTextField
                focus: true
                text: user.email
                placeholderText: i18nc("Example email address", "john.doe@kde.org")
                Kirigami.FormData.label: i18n("Email address:")
            }

            QQC2.Button {
                text: i18n("Change Password")
                onClicked: {
                    changePassword.account = user
                    changePassword.openAndClear()
                }
            }

            Item {
                Layout.preferredHeight: deleteUser.height
            }

            QQC2.Button {
                id: deleteUser

                enabled: !usersDetailPage.user.loggedIn && (!kcm.userModel.rowCount() < 2)

                QQC2.Menu {
                    id: deleteMenu
                    modal: true
                    QQC2.MenuItem {
                        text: i18n("Delete files")
                        icon.name: "edit-delete-shred"
                        onClicked: {
                            if (kcm.deleteUser(usersDetailPage.user.uid, true)) {
                                kcm.pop()
                            }
                        }
                    }
                    QQC2.MenuItem {
                        text: i18n("Keep files")
                        icon.name: "document-multiple"
                        onClicked: {
                            if (kcm.deleteUser(usersDetailPage.user.uid, false)) {
                                kcm.pop()
                            }
                        }
                    }
                }
                text: i18n("Delete User…")
                icon.name: "edit-delete"
                onClicked: deleteMenu.open()
            }
        }
    }

    Kirigami.OverlaySheet {
        id: picturesSheet

        title: i18n("Change Avatar")

        QQC2.SwipeView {
            id: stackSwitcher
            interactive: false

            Layout.preferredWidth: usersDetailPage.width - (Kirigami.Units.largeSpacing*4)
            Keys.onEscapePressed: {
                picturesSheet.close()
                event.accepted = true
            }
            ColumnLayout {
                id: cols
                GridLayout {
                    id: picturesColumn

                    rowSpacing: Kirigami.Units.smallSpacing
                    columnSpacing: Kirigami.Units.smallSpacing
                    columns: {
                        // subtract gridunit from stackswticher width to roughly compensate for slight overlap on tightly fit grids
                        return Math.floor((stackSwitcher.width - Kirigami.Units.gridUnit) / ((Kirigami.Units.gridUnit * 6) + picturesColumn.columnSpacing))
                    }

                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    Layout.rightMargin: Kirigami.Units.largeSpacing

                    QQC2.Button {
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 6
                        Layout.preferredWidth: Layout.preferredHeight

                        ColumnLayout {
                            anchors.centerIn: parent

                            Kirigami.Icon {
                                width: Kirigami.Units.gridUnit * 4
                                height: Kirigami.Units.gridUnit * 4
                                source: "document-open"

                                Layout.alignment: Qt.AlignHCenter
                            }
                            QQC2.Label {
                                text: i18n("Choose File…")

                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignBottom
                                fontSizeMode: Text.HorizontalFit
                                wrapMode: Text.Wrap
                                Layout.fillWidth: true
                                Layout.maximumWidth: Kirigami.Units.gridUnit * 5
                            }
                        }

                        onClicked: fileDialog.open()
                    }

                    QQC2.Button {
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 6
                        Layout.preferredWidth: Layout.preferredHeight

                        Kirigami.Heading {
                            anchors.centerIn: parent
                            font.pixelSize: Kirigami.Units.gridUnit * 4
                            text: kcm.initializeString(user.realName)
                        }

                        onClicked: stackSwitcher.currentIndex = 1
                    }

                    Repeater {
                        model: kcm.avatarFiles
                        QQC2.Button {
                            Layout.preferredHeight: Kirigami.Units.gridUnit * 6
                            Layout.preferredWidth: Layout.preferredHeight

                            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                            QQC2.ToolTip.text: modelData
                            Accessible.name: modelData

                            Image {
                                id: imgDelegate
                                visible: false
                                smooth: true
                                mipmap: true
                                sourceSize.width: Kirigami.Units.gridUnit * 5
                                sourceSize.height: Kirigami.Units.gridUnit * 5
                                source: modelData

                                Accessible.ignored: true
                            }

                            Kirigami.ShadowedTexture {
                                radius: width / 2
                                anchors.centerIn: parent
                                width: Kirigami.Units.gridUnit * 5
                                height: Kirigami.Units.gridUnit * 5

                                source: imgDelegate
                            }

                            onClicked: {
                                picturesSheet.close()
                                usersDetailPage.oldImage = usersDetailPage.user.face
                                usersDetailPage.user.face = imgDelegate.source
                                usersDetailPage.overrideImage = true
                            }
                        }
                    }
                }
            }
            ColumnLayout {
                id: initialPictures
                GridLayout {
                    id: initialsColumn

                    rowSpacing: Kirigami.Units.smallSpacing
                    columnSpacing: Kirigami.Units.smallSpacing
                    columns: {
                        return Math.floor((stackSwitcher.width) / (Kirigami.Units.gridUnit * 6))
                    }

                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    Layout.rightMargin: Kirigami.Units.largeSpacing

                    QQC2.Button {
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 6
                        Layout.preferredWidth: Layout.preferredHeight

                        ColumnLayout {
                            anchors.centerIn: parent

                            Kirigami.Icon {
                                width: Kirigami.Units.gridUnit * 4
                                height: Kirigami.Units.gridUnit * 4
                                source: "go-previous"

                                Layout.alignment: Qt.AlignHCenter
                            }
                        }

                        onClicked: stackSwitcher.currentIndex = 0
                    }

                    Repeater {
                        model: [
                            {"name": i18n("Feisty Flamingo"),  "color": "#E93A9A", "dark": true},
                            {"name": i18n("Dragon's Fruit"),   "color": "#E93D58", "dark": true},
                            {"name": i18n("Sweet Potato"),     "color": "#E9643A", "dark": true},
                            {"name": i18n("Ambient Amber"),    "color": "#EF973C", "dark": false},
                            {"name": i18n("Sparkle Sunbeam"),  "color": "#E8CB2D", "dark": false},
                            {"name": i18n("Lemon-Lime"),       "color": "#B6E521", "dark": false},
                            {"name": i18n("Verdant Charm"),    "color": "#3DD425", "dark": false},
                            {"name": i18n("Mellow Meadow"),    "color": "#00D485", "dark": false},
                            {"name": i18n("Tepid Teal"),       "color": "#00D3B8", "dark": false},
                            {"name": i18n("Plasma Blue"),      "color": "#3DAEE9", "dark": true},
                            {"name": i18n("Pon Purple"),       "color": "#B875DC", "dark": true},
                            {"name": i18n("Bajo Purple"),      "color": "#926EE4", "dark": true},
                            {"name": i18n("Burnt Charcoal"),   "color": "#232629", "dark": true},
                            {"name": i18n("Paper Perfection"), "color": "#EEF1F5", "dark": false},
                            {"name": i18n("Cafétera Brown"),   "color": "#CB775A", "dark": false},
                            {"name": i18n("Rich Hardwood"),    "color": "#6A250E", "dark": true}
                        ]
                        delegate: QQC2.Button {
                            Layout.preferredHeight: Kirigami.Units.gridUnit * 6
                            Layout.preferredWidth: Layout.preferredHeight

                            Rectangle {
                                id: colourRectangle

                                anchors.fill: parent
                                anchors.margins: Kirigami.Units.smallSpacing
                                color: modelData.color

                                Rectangle {
                                    anchors.fill: parent
                                    gradient: Gradient {
                                        GradientStop { position: 0.0; color: "transparent" }
                                        GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.3) }
                                    }
                                }

                                Kirigami.Heading {
                                    anchors.centerIn: parent
                                    color: modelData.dark ? "white" : "black"
                                    font.pixelSize: Kirigami.Units.gridUnit * 4
                                    text: kcm.initializeString(user.realName)
                                }
                            }

                            onClicked: {
                                colourRectangle.grabToImage(function(result) {
                                    picturesSheet.close()
                                    let uri = kcm.plonkImageInTempfile(result.image)
                                    if (uri != "") {
                                        usersDetailPage.oldImage = usersDetailPage.user.face
                                        usersDetailPage.user.face = uri
                                        usersDetailPage.overrideImage = true
                                    }
                                })
                            }
                        }
                    }
                }
            }
        }
    }

    ChangePassword { id: changePassword; account: user }

    ChangeWalletPassword { id: changeWalletPassword }
}
