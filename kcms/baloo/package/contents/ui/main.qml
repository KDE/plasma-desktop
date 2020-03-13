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
            enabled: !kcm.balooSettings.isImmutable("indexingEnabled")
            checked: kcm.balooSettings.indexingEnabled
            onCheckStateChanged: {
                kcm.balooSettings.indexingEnabled = checked
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Item {
                width: units.largeSpacing
            }

            ColumnLayout {
                QQC2.CheckBox {
                    id: indexFileContents
                    text: i18n("Also index file content")
                    enabled: fileSearchEnabled.checked && !kcm.balooSettings.isImmutable("onlyBasicIndexing")
                    checked: !kcm.balooSettings.onlyBasicIndexing
                    onCheckStateChanged: kcm.balooSettings.onlyBasicIndexing = !checked
                }
                QQC2.CheckBox {
                    id: indexHiddenFolders
                    text: i18n("Index hidden files and folders")
                    enabled: fileSearchEnabled.checked && !kcm.balooSettings.isImmutable("indexHiddenFolders")
                    checked: kcm.balooSettings.indexHiddenFolders
                    onCheckStateChanged: kcm.balooSettings.indexHiddenFolders = checked
                }
            }
        }

        Item {
            Layout.preferredHeight: Kirigami.Units.gridUnit
        }
        QQC2.Label {
            text: i18n("Folder specific configuration:")
        }

        QQC2.ScrollView {
            id: bgObject
            Component.onCompleted: bgObject.background.visible = true
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: directoryConfigList
                clip: true
                currentIndex: -1

                model: kcm.filteredModel
                delegate: directoryConfigDelegate
            }
        }

        QQC2.Button {
            Layout.alignment: Qt.AlignRight
            id: addFolder
            icon.name: "folder-add"
            text: i18n("Add folder configuration...")
            onClicked: fileDialogLoader.active = true
        }
    }

    Component {
        id: directoryConfigDelegate
        Kirigami.SwipeListItem {
            id: listItem
            onClicked: {
                directoryConfigList.currentIndex = index
            }
            property int iconSize: Kirigami.Units.iconSizes.smallMedium
            property bool selected: directoryConfigList.currentIndex === index

            RowLayout {
                spacing: units.smallSpacing

                Kirigami.Icon {
                    source: model.enableIndex ? "search" : "list-remove"
                    height: listItem.iconSize
                    width: listItem.iconSize
                }

                ColumnLayout {
                    RowLayout {
                        spacing: units.smallSpacing

                        Kirigami.Icon {
                            source: model.decoration
                            height: listItem.iconSize
                            width: listItem.iconSize
                        }
                        QQC2.Label {
                            text: model.folder
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                    QQC2.Label {
                        text: (model.enableIndex ? i18n("%1 is included.", model.url)
                                                 : i18n("%1 is excluded.", model.url))
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        opacity: listItem.hovered ? 0.8 : 0.6
                        visible: listItem.selected
                    }
                }

                QQC2.ToolButton {
                    visible: listItem.hovered && listItem.actionsVisible
                    height: listItem.iconSize
                    icon.name: "search"
                    text: model.enableIndex ? i18n("Disable indexing") : i18n("Enable indexing")
                    onClicked: {
                        model.enableIndex = !model.enableIndex
                    }
                }
            }

            actions: [
                Kirigami.Action {
                    id: removeFolder
                    enabled: model.deletable
                    icon.name: "user-trash"
                    tooltip: i18n("Delete entry")
                    onTriggered: {
                        kcm.filteredModel.removeFolder(index)
                    }
                }
            ]
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
