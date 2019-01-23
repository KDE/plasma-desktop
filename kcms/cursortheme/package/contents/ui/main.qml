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
import org.kde.kconfig 1.0 // for KAuthorized
import org.kde.kcm 1.1 as KCM

import org.kde.private.kcm_cursortheme 1.0

KCM.GridViewKCM {
    KCM.ConfigModule.quickHelp: i18n("This module lets you choose the mouse cursor theme.")

    view.model: kcm.cursorsModel
    view.delegate: Delegate {}
    view.onCurrentIndexChanged: {
        kcm.selectedThemeRow = view.currentIndex;
        view.positionViewAtIndex(view.currentIndex, view.GridView.Beginning);
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

    Connections {
        target: kcm
        onSelectedThemeRowChanged: view.currentIndex = kcm.selectedThemeRow;
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
                enabled: kcm.canResize
                QtControls.Label {
                    text: i18n("Size:")
                }
                QtControls.ComboBox {
                    id: sizeCombo
                    model: kcm.sizesModel
                    textRole: "display"
                    onActivated: {
                        kcm.selectedSizeRow = sizeCombo.currentIndex
                    }

                    Connections {
                        target: kcm
                        onSelectedSizeRowChanged: {
                            sizeCombo.currentIndex = kcm.selectedSizeRow
                        }
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
                            }
                        }
                    }
                }
            }
            RowLayout {
                parent: footerLayout.x + footerLayout.width - comboLayout.width > width ? row1 : row2
                QtControls.Button {
                    icon.name: "document-import"
                    text: i18n("&Install From File...")
                    onClicked: fileDialogLoader.active = true;
                    enabled: kcm.canInstall
                }
                QtControls.Button {
                    icon.name: "get-hot-new-stuff"
                    text: i18n("&Get New Cursors...")
                    onClicked: kcm.getNewClicked();
                    enabled: kcm.canInstall
                    visible: KAuthorized.authorize("ghns")
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

