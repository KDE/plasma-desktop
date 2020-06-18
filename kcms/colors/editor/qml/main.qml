/*
 * Copyright 2020 Noah Davis <noahadvs@gmail.com>
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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.newstuff 1.0 as KNS

// import org.kde.kcolorschemeeditor 1.0

Kirigami.ApplicationWindow {
    id: root
    // TODO: use real colorscheme name
    title: "Breeze Dark"
    width: Kirigami.Units.gridUnit * 64
    height: Kirigami.Units.gridUnit * 48

    /*header: QQC2.ToolBar {
        id: toolbarArea
        padding: 0
        Kirigami.SearchField {
            id: searchField
            anchors {
                left: parent.left
                right: toolSeparator.left
                top: parent.top
                bottom: parent.bottom
                leftMargin: Kirigami.Units.smallSpacing
                topMargin: anchors.leftMargin
                bottomMargin: anchors.leftMargin
            }
//             onAccepted: console.log("Search text is " + searchField.text)
        }
        QQC2.ToolSeparator {
            id: toolSeparator
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            x: sidebarSeparator.x - toolSeparator.leftPadding
        }
        Kirigami.ActionToolBar {
            id: actionToolBar
            anchors {
                left: toolSeparator.right
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
            actions: [
                Kirigami.Action {
                    iconName: "color-picker"
                    text: i18n("Locate Color Role")
                    shortcut: "Ctrl+L"
                    tooltip: i18n("Locate a color role by clicking on a part of the preview window") + " (" + shortcut + ")"
                },
                Kirigami.Action {
                    iconName: "document-save"
                    text: i18n("Save")
                    shortcut: "Ctrl+S"
                    tooltip: text + " (" + shortcut + ")"
                },
                Kirigami.Action {
                    iconName: "document-save-as"
                    text: i18n("Save As...")
                    shortcut: "Ctrl+Shift+S"
                    tooltip: text + " (" + shortcut + ")"
                },
                Kirigami.Action {
                    iconName: "edit-undo"
                    text: i18n("Undo")
                    shortcut: "Ctrl+Z"
                },
                Kirigami.Action {
                    iconName: "edit-redo"
                    text: i18n("Redo")
                    shortcut: "Ctrl+Shift+Z"
                },
                Kirigami.Action {
                    iconName: "edit-reset"
                    text: i18n("Reset All")
                }
                // Upload button from old kcolorscheme UI isn't available for QML
            ]
        }
    }*/
    pageStack.initialPage: [colorListComponent, previewAreaComponent]
    
    Component {
        id: colorListComponent
        ColorList {
            id: colorList
            Layout.fillHeight: true
            Layout.minimumWidth: this.implicitWidth
            Layout.preferredWidth: Kirigami.Units.gridUnit * 16
        }
    }
    Component {
        id: previewAreaComponent
        PreviewArea {
            id: previewArea
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: this.implicitWidth
            Layout.minimumHeight: this.implicitHeight
            Layout.preferredWidth: root.width * 0.75
        }
    }
    
    /*
    RowLayout {
        id: contentRowLayout
        anchors.fill: parent
        spacing: 0
        
        Kirigami.Separator {
            id: sidebarSeparator
            Layout.fillHeight: true
        }
        
    }
    */
}
