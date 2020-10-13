/***************************************************************************
 *   Copyright (C) 2020 by Nicolas Fella <nicolas.fella@gmx.de             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

import QtQuick 2.10
import QtQuick.Controls 2.10
import QtQuick.Layouts 1.11
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Dialogs 1.3
import org.kde.kcm 1.2 as KCM
import org.kde.plasma.kcm.autostart 1.0

KCM.ScrollViewKCM {

    id: root

    implicitHeight: Kirigami.Units.gridUnit * 28
    implicitWidth: Kirigami.Units.gridUnit * 28

    header: Kirigami.InlineMessage {
        id: errorMessage
        type: Kirigami.MessageType.Error
        showCloseButton: true

        Connections {
            target: kcm.model
            function onError(message) {
                errorMessage.visible = true
                errorMessage.text = message
            }
        }
    }

    view: ListView {
        clip: true
        model: kcm.model

        delegate: Kirigami.SwipeListItem {

            Item {
                Kirigami.Icon {
                    id: appIcon
                    source: model.iconName
                    width: Kirigami.Units.iconSizes.medium
                    height: Kirigami.Units.iconSizes.medium
                }

                Label {
                    height: appIcon.height
                    text: model.name
                    elide: Text.ElideRight
                    anchors.left: appIcon.right
                    anchors.leftMargin: Kirigami.Units.largeSpacing
                    anchors.right: parent.right
                }
            }

            actions: [
                Kirigami.Action {
                    text: i18n("Properties")
                    icon.name: "document-properties"
                    onTriggered: kcm.model.editApplication(model.index, root)
                    visible: model.source === AutostartModel.XdgAutoStart
                },
                Kirigami.Action {
                    text: i18n("Remove")
                    icon.name: "list-remove"
                    onTriggered: kcm.model.removeEntry(model.index)
                }
            ]
        }

        section.property: "source"
        section.delegate: Kirigami.ListSectionHeader {
            text: {
                if (section == AutostartModel.XdgAutoStart) {
                    return i18n("Applications")
                }
                if (section == AutostartModel.XdgScripts || section == AutostartModel.PlasmaStart) {
                    return i18n("Login Scripts")
                }
                if (section == AutostartModel.PlasmaShutdown) {
                    return i18n("Logout Scripts")
                }
            }
        }
    }

    footer: Row {
        spacing: Kirigami.Units.largeSpacing

        Loader {
            id: startupFileDialogLoader

            active: false

            sourceComponent: FileDialog {
                id: startupFileDialog
                title: i18n("Choose Startup Script")
                folder: shortcuts.home
                selectMultiple: false
                onAccepted: {
                    kcm.model.addScript(startupFileDialog.fileUrl, AutostartModel.XdgScripts)
                    startupFileDialogLoader.active = false
                }

                onRejected: startupFileDialogLoader.active = false

                Component.onCompleted: open()
            }
        }

        Loader {
            id: logoutFileDialogLoader

            active: false

            sourceComponent: FileDialog {
                id: logoutFileDialog
                title: i18n("Choose Logout Script")
                folder: shortcuts.home
                selectMultiple: false
                onAccepted: {
                    kcm.model.addScript(logoutFileDialog.fileUrl, AutostartModel.PlasmaShutdown)
                    logoutFileDialogLoader.active = false
                }

                onRejected: logoutFileDialogLoader.active = false

                Component.onCompleted: open()
            }
        }

        Button {
            id: menuButton

            icon.name: "list-add"
            text: i18n("Add...")

            checkable: true
            checked: menu.opened

            onPressed: {
                // Appear above the button, not below it, since the button is at
                // the bottom of the window and QQC2 items can't leave the window

                // HACK: since we want to position the menu above the button,
                // we need to know the menu's height, but it only has a height
                // after the first time it's been shown, so until then, we need
                // to provide an artificially-synthesized-and-hopefully-good-enough
                // height value
                var menuHeight = menu.height && menu.height > 0 ? menu.height : Kirigami.Units.gridUnit * 3
                menu.popup(menuButton, 0, -menuHeight)
            }
        }

        Menu {
            id: menu

            modal: true
            dim: false

            MenuItem {
                text: i18n("Add Application...")
                icon.name: "list-add"

                onClicked: kcm.model.showApplicationDialog(root)
            }
            MenuItem {
                text: i18n("Add Login Script...")
                icon.name: "list-add"

                onClicked: startupFileDialogLoader.active = true
            }
            MenuItem {
                text: i18n("Add Logout Script...")
                icon.name: "list-add"

                onClicked: logoutFileDialogLoader.active = true
            }
        }
    }
}
