/*
 * Copyright 2020 Mikhail Zolotukhin <zomial@protonmail.com>
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

import QtQuick 2.7
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0 as QtDialogs
import QtQuick.Controls 2.10 as QtControls
import org.kde.kirigami 2.10 as Kirigami
import org.kde.private.kcms.style 1.0 as Private
import org.kde.newstuff 1.62 as NewStuff
import org.kde.kcm 1.2 as KCM

Kirigami.Page {
    id: gtkStylePage
    title: i18n("GNOME/GTK Application Style")

    ColumnLayout {
        anchors.fill: parent

        Kirigami.InlineMessage {
            id: infoLabel
            Layout.fillWidth: true

            showCloseButton: true
            visible: false

            Connections {
                target: kcm.gtkPage
                onShowErrorMessage: {
                    infoLabel.type = Kirigami.MessageType.Error;
                    infoLabel.text = message;
                    infoLabel.visible = true;
                }
            }
        }

        Kirigami.FormLayout {
            wideMode: true

            Row {
                Kirigami.FormData.label: i18n("GTK theme:")

                Flow {
                    spacing: Kirigami.Units.smallSpacing

                    QtControls.ComboBox {
                        id: gtkThemeCombo
                        model: kcm.gtkPage.gtkThemesModel
                        currentIndex: model.findThemeIndex(kcm.gtkPage.gtkThemeFromConfig())
                        onCurrentTextChanged: function() {
                            model.selectedTheme = currentText
                            gtkRemoveButton.enabled = model.selectedThemeRemovable()
                        }
                        onActivated: model.setSelectedThemeDirty()
                        textRole: "theme-name"

                        Connections {
                            target: kcm.gtkPage
                            onSelectGtkThemeInCombobox: function(themeName) {
                                gtkThemeCombo.currentIndex = gtkThemeCombo.model.findThemeIndex(themeName)
                            }
                        }
                    }

                    QtControls.Button {
                        id: gtkRemoveButton
                        icon.name: "edit-delete"
                        onClicked: gtkThemeCombo.model.removeSelectedTheme()
                    }

                    QtControls.Button {
                        icon.name: "preview"
                        text: i18n("Preview...")
                        onClicked: kcm.gtkPage.showGtkPreview()
                        visible: kcm.gtkPage.gtkPreviewAvailable()
                    }

                }
            }

        }

        Item {
            Layout.fillHeight: true
        }

        RowLayout {
            Item {
                Layout.fillWidth: true
            }

            QtControls.Button {
                icon.name: "document-import"
                text: i18n("Install from File...")
                onClicked: fileDialogLoader.active = true
            }

            NewStuff.Button {
                id: gtkNewStuffButton
                text: i18n("Get New GNOME/GTK Application Styles...")
                configFile: "gtk_themes.knsrc"
                viewMode: NewStuff.Page.ViewMode.Preview
                onChangedEntriesChanged: kcm.gtkPage.onGhnsEntriesChanged(gtkNewStuffButton.changedEntries);
            }
        }
    }

    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: QtDialogs.FileDialog {
            title: i18n("Select GTK Theme Archive")
            folder: shortcuts.home
            nameFilters: [ i18n("GTK Theme Archive (*.tar.xz *.tar.gz *.tar.bz2)") ]
            Component.onCompleted: open()
            onAccepted: {
                kcm.gtkPage.installGtkThemeFromFile(fileUrls[0])
                fileDialogLoader.active = false
            }
            onRejected: {
                fileDialogLoader.active = false
            }
        }
    }
}
