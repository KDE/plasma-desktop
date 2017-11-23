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
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2 as QQC1
import QtQuick.Controls 2.2 as QtControls
import org.kde.kirigami 2.2 as Kirigami
import org.kde.kcm 1.0
import org.kde.kcmcontrols 1.0 as KCMControls

import org.kde.private.kcm_cursortheme 1.0

KCMControls.GridViewKCM {

    
    ConfigModule.quickHelp: i18n("This module lets you configure the mouse cursor theme used.")

    view.model: kcm.cursorsModel
    view.delegate: Delegate {}
    view.onCurrentIndexChanged: {
        kcm.selectedThemeRow = view.currentIndex;
        view.positionViewAtIndex(view.currentIndex, view.GridView.Beginning);
    }

    Connections {
        target: kcm
        onSelectedThemeRowChanged: view.currentIndex = kcm.selectedThemeRow;
    }

    footer: ColumnLayout {
        id: footerLayout
        RowLayout {
            id: row1
                //spacer
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

            RowLayout {
                id: comboLayout
                QtControls.Label {
                    text: i18n("Size:")
                }
                QtControls.ComboBox {
                    id: sizeCombo
                    currentIndex: 0
                    onCountChanged: currentIndex = 0
                    model: kcm.sizesModel
                    textRole: "display"
                    onCurrentTextChanged: {
                        kcm.preferredSize = isNaN(parseInt(sizeCombo.currentText)) ? 0 : parseInt(sizeCombo.currentText);
                    }
                }
            }
            RowLayout {
                parent: footerLayout.x + footerLayout.width - comboLayout.width > width ? row1 : row2
                //TODO: port to QQC2 buttons as soon they have icons
                QQC1.Button {
                    iconName: "document-import"
                    text: i18n("&Install From File...")
                    onClicked: kcm.installClicked();
                    enabled: kcm.canInstall
                }
                QQC1.Button {
                    iconName: "get-hot-new-stuff"
                    text: i18n("&Get New Theme...")
                    onClicked: kcm.getNewClicked();
                    enabled: kcm.canInstall
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
}

