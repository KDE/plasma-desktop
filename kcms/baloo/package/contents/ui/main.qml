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
import org.kde.kcm 1.3 as KCM

KCM.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 42
    implicitHeight: Kirigami.Units.gridUnit * 25

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
            checked: kcm.balooSettings.indexingEnabled
            onCheckStateChanged: {
                kcm.balooSettings.indexingEnabled = checked
            }

            KCM.SettingStateBinding {
                configObject: kcm.balooSettings
                settingName: "indexingEnabled"
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
                    checked: !kcm.balooSettings.onlyBasicIndexing
                    onCheckStateChanged: kcm.balooSettings.onlyBasicIndexing = !checked

                    KCM.SettingStateBinding {
                        configObject: kcm.balooSettings
                        settingName: "onlyBasicIndexing"
                        extraEnabledConditions: fileSearchEnabled.checked
                    }
                }
                QQC2.CheckBox {
                    id: indexHiddenFolders
                    text: i18n("Index hidden files and folders")
                    checked: kcm.balooSettings.indexHiddenFolders
                    onCheckStateChanged: kcm.balooSettings.indexHiddenFolders = checked

                    KCM.SettingStateBinding {
                        configObject: kcm.balooSettings
                        settingName: "indexHiddenFolders"
                        extraEnabledConditions: fileSearchEnabled.checked
                    }
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
            id: menuButton

            Layout.alignment: Qt.AlignRight

            icon.name: "folder-add"
            text: i18n("Add folder configuration...")

            checkable: true
            checked: menu.opened

            onClicked: {
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

        QQC2.Menu {
            id: menu

            modal: true

            QQC2.MenuItem {
                text: i18n("Start indexing a folder...")
                icon.name: "list-add"

                onClicked: {
                    fileDialogLoader.included = true
                    fileDialogLoader.active = true
                }
            }
            QQC2.MenuItem {
                text: i18n("Stop indexing a folder...")
                icon.name: "list-remove"

                onClicked: {
                    fileDialogLoader.included = false
                    fileDialogLoader.active = true
                }
            }
        }
    }

    Component {
        id: directoryConfigDelegate
        Kirigami.AbstractListItem {
            id: listItem

            // Store this as a property so we can access it within the combobox,
            // which also has a `model` property
            property var indexingModel: model

            // There's no need for a list item to ever be selected
            highlighted: false
            hoverEnabled: false

            contentItem: RowLayout {
                spacing: units.smallSpacing

                // The folder's icon
                Kirigami.Icon {
                    source: indexingModel.decoration

                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredWidth: Layout.preferredHeight
                }

                // The folder's path
                QQC2.Label {
                    text: indexingModel.folder
                    elide: Text.ElideRight

                    Layout.fillWidth: true
                }

                // What kind of indexing to do for the folder
                QQC2.ComboBox {
                    id: indexingOptionsCombobox

                    property bool indexingDisabled: !indexingModel.enableIndex
                    property bool fullContentIndexing: indexingModel.enableIndex

                    model: [
                        i18n("Not indexed"),
                        i18n("Indexed")
                    ]

                    // Intentionally not a simple ternary to facilitate adding
                    // more conditions in the future
                    currentIndex: {
                        if (indexingDisabled) return 0
                        if (fullContentIndexing) return 1
                    }

                    onActivated: {
                        // New value is "Not indexed"
                        if (indexingOptionsCombobox.currentIndex === 0 && fullContentIndexing) {
                            indexingModel.enableIndex = false
                        // New value is "Full content indexing"
                        } else if (indexingOptionsCombobox.currentIndex === 1 && indexingDisabled) {
                            indexingModel.enableIndex = true
                        }
                    }
                }

                // Delete button to remove this folder entry
                QQC2.Button {
                    enabled: model.deletable

                    icon.name: "edit-delete"

                    onClicked: kcm.filteredModel.removeFolder(index)

                    QQC2.ToolTip {
                        text: i18n("Delete entry")
                    }
                }
            }
        }
    }

    Loader {
        id: fileDialogLoader

        property bool included: false

        active: false

        sourceComponent: QtDialogs.FileDialog {
            title: i18n("Select a folder to filter")
            folder: shortcuts.home
            selectFolder: true

            onAccepted: {
                kcm.filteredModel.addFolder(fileUrls[0], included)
                fileDialogLoader.active = false
            }

            onRejected: {
                fileDialogLoader.active = false
            }

            Component.onCompleted: open()
        }
    }
}
