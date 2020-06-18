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

import QtQuick 2.13
import QtQuick.Layouts 1.13
import QtQuick.Controls 2.13 as QQC2
import org.kde.kirigami 2.13 as Kirigami
import "previewwindowcontentscomponents"

Kirigami.ApplicationItem {
    id: root
    clip: true
    Kirigami.Theme.inherit: false
    
    /*header: QQC2.ToolBar {
        anchors {
            left: parent.left
            right: parent.right
        }
        Kirigami.ActionToolBar {
            anchors.fill: parent
            actions: [
                Kirigami.Action {
                    iconName: "document-save"
                    text: i18n("Action 1")
                    shortcut: "Ctrl+S"
                    tooltip: root.fakeControlToolTip
                },
                Kirigami.Action {
                    iconName: "edit-undo"
                    text: i18n("Action 2")
                    shortcut: "Ctrl+Z"
                    tooltip: root.fakeControlToolTip
                },
                Kirigami.Action {
                    iconName: "edit-redo"
                    text: i18n("Action 3")
                    shortcut: "Ctrl+Shift+Z"
                    tooltip: root.fakeControlToolTip
                }
            ]
        }
    }*/
    
    readonly property string fakeControlToolTip: i18n("This control does nothing.")
    pageStack.defaultColumnWidth: Kirigami.Units.gridUnit * 7
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.ToolBar
//    pageStack.globalToolBar.toolbarActionAlignment: Qt.AlignLeft
    pageStack.initialPage: [sideBarComponent, mainPageComponent]
    
    Component {
        id: mainPageComponent
        MainPage {}
    }
    
    Component {
        id: sideBarComponent
        SideBar {}
    }
}
