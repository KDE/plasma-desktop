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
import org.kde.kirigami 2.12 as Kirigami

Kirigami.ShadowedRectangle {
    id: root
    
    implicitWidth: titleBar.implicitWidth
    implicitHeight: root.implicitWidth
    
    property real shadowSize: 32
    property real shadowYOffset: shadowSize / 4
    
    Kirigami.Theme.inherit: false
    //TODO: replace colors with colors from the specified colorscheme
    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    color: Kirigami.Theme.backgroundColor
    radius: titleBar.titleBarCornerRadius
//     corners {
//         topLeftRadius: titleBar.titleBarCornerRadius
//         topRightRadius: titleBar.titleBarCornerRadius
//     }
    shadow {
        size: shadowSize
        yOffset: shadowYOffset
        color: Qt.rgba(0, 0, 0, 0.9)
    }    
    
    PreviewTitleBar {
        id: titleBar
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
    }
    
    PreviewWindowContents {
        anchors {
            top: titleBar.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
//             margins: Kirigami.Units.smallSpacing
        }
    }
}
