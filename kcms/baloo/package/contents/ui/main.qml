/*
 * Copyright 2018 Tomaz Canabrava <tcanabrava@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.11 as QQC2
import QtQuick.Dialogs 1.2 as QtDialogs
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.1 as KCM

KCM.SimpleKCM {
    id: root

    implicitHeight: Kirigami.Units.gridUnit * 22

    KCM.ConfigModule.quickHelp: i18n("This module lets you configure the file indexer and search functionality.")
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing

        QQC2.Label {
            text: i18n("File Search helps you quickly locate all your files based on their content.")
        }

        QQC2.CheckBox {
            id: fileSearchEnabled
            text: i18n("Enable File Search")
            checked: kcm.indexing
            onCheckStateChanged: {
                kcm.indexing = checked
            }
        }

        QQC2.CheckBox {
            id: indexFileContents
            text: i18n("Also index file content")
            enabled: fileSearchEnabled.checked
            checked: kcm.fileContents
            onCheckStateChanged: kcm.fileContents = checked
        }
        Item {
            Layout.preferredHeight: Kirigami.Units.gridUnit
        }
        QQC2.Label {
            text: i18n("Do not search in these locations:")
        }

        QQC2.ScrollView {
            id: bgObject
            Component.onCompleted: bgObject.background.visible = true
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: fileExcludeList

                clip: true
                model: kcm.filteredModel
                delegate: Kirigami.BasicListItem {
                    icon: model.decoration
                    label: model.folder
                    onClicked: fileExcludeList.currentIndex = index
                }
            }
        }

        RowLayout {
            QQC2.Button {
                id: addFolder
                icon.name: "list-add"
                onClicked: fileDialogLoader.active = true
            }

            QQC2.Button{
                id: removeFolder
                icon.name: "list-remove"
                enabled: fileExcludeList.currentIndex !== -1
                onClicked: {
                    kcm.filteredModel.removeFolder(fileExcludeList.currentIndex)
                }
            }
        }
    }

    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: QtDialogs.FileDialog {
            title: i18n("Select a folder to filter")
            folder: shortcuts.home
            selectFolder: true

            onAccepted: {
                kcm.filteredModel.addFolder(fileUrls[0])
                fileDialogLoader.active = false
            }

            onRejected: {
                fileDialogLoader.active = false
            }

            Component.onCompleted: open()
        }
    }
}
