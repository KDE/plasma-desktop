/* SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.12 as Kirigami

Kirigami.ScrollablePage {
    id: root
    implicitWidth: Kirigami.Units.gridUnit * 10
    padding: 0
    
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    
    titleDelegate: Kirigami.SearchField {
        Layout.fillWidth: true
    }
    
    /*actions {
        main: Kirigami.Action {
            displayComponent: Kirigami.SearchField {
                Layout.fillWidth: true
            }
        }
    }*/
    
    ListModel {
        id: listModel
        ListElement {
            color: "#000000"
            colorRole: "Normal Background"
            colorSet: "Window"
        }
        ListElement {
            color: "#ffffff"
            colorRole: "Normal Text"
            colorSet: "View"
        }
        ListElement {
            color: "#3daee9"
            colorRole: "Normal Background"
            colorSet: "Selection"
        }
    }

    ListView {
        id: listView
        model: listModel
        section.property: "colorSet"
        section.delegate: Kirigami.ListSectionHeader {
            label: section
        }
        delegate: Kirigami.BasicListItem {
            separatorVisible: false
            label: model.colorRole
            subtitle: "short description"
            
            Rectangle {
                id: colorRoleOutlineRect
                color: Kirigami.Theme.backgroundColor
                border.color: Kirigami.ColorUtils.linearInterpolation(root.background.color, Kirigami.Theme.textColor, 0.25)
                border.width: 1
                radius: 3
                height: Kirigami.Units.iconSizes.medium
                width: colorRoleRect.height * 2
                
                Rectangle {
                    id: colorRoleRect
                    color: model.color
                    radius: parent.radius - anchors.margins
                    anchors {
                        fill: parent
                        margins: parent.border.width * 2
                    }
                }
            }
        }
    }
}

