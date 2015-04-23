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

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.0 as QtControls
//We need units from it
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kcm 1.0

import org.kde.private.kcm_cursortheme 1.0

Item {
    implicitWidth: units.gridUnit * 20
    implicitHeight: units.gridUnit * 20

    ConfigModule.quickHelp: i18n("This module lets you configure the mouse cursor theme used.")

    SystemPalette {id: syspal}

    Connections {
        target: kcm
        onSelectedThemeRowChanged: view.currentIndex = kcm.selectedThemeRow;
    }

    RowLayout {
        anchors.fill: parent
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            PreviewWidget {
                id: previewWidget
                Layout.alignment: Qt.AlignHCenter
                Layout.minimumWidth: implicitWidth
                Layout.minimumHeight: implicitHeight
                Layout.maximumWidth: Layout.minimumWidth
                Layout.maximumHeight: Layout.minimumHeight
                themeModel: kcm.cursorsModel
                currentIndex: view.currentIndex
                //The ComboBox component is a disaster
                currentSize: parseInt(sizeCombo.currentText) !== NaN ? parseInt(sizeCombo.currentText) : 0
            }
            QtControls.ScrollView {
                Rectangle {
                    anchors.fill: parent
                    z: -1
                }
                Layout.fillWidth: true
                Layout.fillHeight: true
                frameVisible: true
                ListView {
                    id: view
                    model: kcm.cursorsModel
                    highlightMoveDuration: 0
                    highlightResizeDuration: 0
                    onCurrentIndexChanged: {
                        kcm.selectedThemeRow = currentIndex;
                    }
                    delegate: Delegate {}
                }
            }
        }

        ColumnLayout {

            QtControls.ComboBox {
                id: sizeCombo
                Layout.minimumHeight: previewWidget.height
                Layout.fillWidth: true
                model: kcm.sizesModel
                textRole: "display"
                enabled: kcm.canResize
                onCurrentTextChanged: {
                    kcm.preferredSize = parseInt(sizeCombo.currentText) !== NaN ? parseInt(sizeCombo.currentText) : 0
                }
            }
            QtControls.Button {
                Layout.fillWidth: true
                iconName: "get-hot-new-stuff"
                text: i18n("Get New Theme...")
                onClicked: kcm.getNewClicked();
                enabled: kcm.canInstall
            }
            QtControls.Button {
                Layout.fillWidth: true
                iconName: "document-import"
                text: i18n("Install From File...")
                onClicked: kcm.installClicked();
                enabled: kcm.canInstall
            }
            QtControls.Button {
                Layout.fillWidth: true
                iconName: "edit-delete"
                text: i18n("Remove Theme")
                onClicked: kcm.removeClicked();
                enabled: kcm.canRemove
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
