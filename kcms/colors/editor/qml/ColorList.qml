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

Kirigami.ScrollablePage {
    id: root
    
    implicitWidth: Kirigami.Units.gridUnit * 4
    padding: 0
    
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    
    titleDelegate: Kirigami.SearchField {
        Layout.fillWidth: true
    }
    
//     actions {
//         main: Kirigami.Action {
//             displayComponent: Kirigami.SearchField {
//                 Layout.fillWidth: true
//             }
//         }
//     }
    
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

