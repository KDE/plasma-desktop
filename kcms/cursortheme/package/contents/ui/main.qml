/*
   Copyright (c) 2015 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.7
import QtQuick.Window 2.2 // for Screen
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.2 as QtControls
import QtQuick.Dialogs 1.1 as QtDialogs
import org.kde.kirigami 2.4 as Kirigami
import org.kde.newstuff 1.62 as NewStuff
import org.kde.kcm 1.3 as KCM

import org.kde.private.kcm_cursortheme 1.0

KCM.GridViewKCM {
    id: root
    KCM.ConfigModule.quickHelp: i18n("This module lets you choose the mouse cursor theme.")

    view.model: kcm.cursorsModel
    view.delegate: Delegate {}
    view.currentIndex: kcm.cursorThemeIndex(kcm.cursorThemeSettings.cursorTheme);

    view.onCurrentIndexChanged: {
        kcm.cursorThemeSettings.cursorTheme = kcm.cursorThemeFromIndex(view.currentIndex)
        view.positionViewAtIndex(view.currentIndex, view.GridView.Beginning);
    }

    Component.onCompleted: {
        view.positionViewAtIndex(view.currentIndex, GridView.Beginning);
    }

    KCM.SettingStateBinding {
        configObject: kcm.cursorThemeSettings
        settingName: "cursorTheme"
        extraEnabledConditions: !kcm.downloadingFile
    }

    DropArea {
        anchors.fill: parent
        onEntered: {
            if (!drag.hasUrls) {
                drag.accepted = false;
            }
        }
        onDropped: kcm.installThemeFromFile(drop.urls[0])
    }

    footer: ColumnLayout {
        id: footerLayout

        Kirigami.InlineMessage {
            id: infoLabel
            Layout.fillWidth: true

            showCloseButton: true

            Connections {
                target: kcm
                onShowSuccessMessage: {
                    infoLabel.type = Kirigami.MessageType.Positive;
                    infoLabel.text = message;
                    infoLabel.visible = true;
                }
                onShowInfoMessage: {
                    infoLabel.type = Kirigami.MessageType.Information;
                    infoLabel.text = message;
                    infoLabel.visible = true;
                }
                onShowErrorMessage: {
                    infoLabel.type = Kirigami.MessageType.Error;
                    infoLabel.text = message;
                    infoLabel.visible = true;
                }
            }
        }

        RowLayout {
            id: row1
                //spacer
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

            RowLayout {
                id: comboLayout

                KCM.SettingStateBinding {
                    configObject: kcm.cursorThemeSettings
                    settingName: "cursorSize"
                    extraEnabledConditions: kcm.canResize
                }

                QtControls.Label {
                    text: i18n("Size:")
                }
                QtControls.ComboBox {
                    id: sizeCombo
                    model: kcm.sizesModel
                    textRole: "display"
                    currentIndex: kcm.cursorSizeIndex(kcm.cursorThemeSettings.cursorSize);
                    onActivated: {
                        kcm.cursorThemeSettings.cursorSize = kcm.cursorSizeFromIndex(sizeCombo.currentIndex);
                        kcm.preferredSize = kcm.cursorSizeFromIndex(sizeCombo.currentIndex);
                    }

                    delegate: QtControls.ItemDelegate {
                        id: sizeComboDelegate

                        readonly property int size: parseInt(model.display)

                        width: parent.width
                        highlighted: ListView.isCurrentItem
                        text: model.display

                        contentItem: RowLayout {
                            Kirigami.Icon {
                                source: model.decoration
                                smooth: true
                                width: sizeComboDelegate.size / Screen.devicePixelRatio
                                height: sizeComboDelegate.size / Screen.devicePixelRatio
                                visible: valid && sizeComboDelegate.size > 0
                            }

                            QtControls.Label {
                                Layout.fillWidth: true
                                color: sizeComboDelegate.highlighted ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                                text: model[sizeCombo.textRole]
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
            RowLayout {
                parent: footerLayout.x + footerLayout.width - comboLayout.width > width ? row1 : row2
                QtControls.Button {
                    icon.name: "document-import"
                    text: i18n("&Install from File...")
                    onClicked: fileDialogLoader.active = true;
                    enabled: kcm.canInstall
                }
                NewStuff.Button {
                    id: newStuffButton
                    enabled: kcm.canInstall
                    text: i18n("&Get New Cursors...")
                    configFile: "xcursor.knsrc"
                    viewMode: NewStuff.Page.ViewMode.Tiles
                    onChangedEntriesChanged: kcm.ghnsEntriesChanged(newStuffButton.changedEntries);
                }
            }
        }
        RowLayout {
            id: row2
            visible: children.length > 1
            //spacer
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: QtDialogs.FileDialog {
            title: i18n("Open Theme")
            folder: shortcuts.home
            nameFilters: [ i18n("Cursor Theme Files (*.tar.gz *.tar.bz2)") ]
            Component.onCompleted: open()
            onAccepted: {
                kcm.installThemeFromFile(fileUrls[0])
                fileDialogLoader.active = false
            }
            onRejected: {
                fileDialogLoader.active = false
            }
        }
    }
}

