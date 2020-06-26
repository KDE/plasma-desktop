/* SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 */

import QtQuick 2.13
import QtQuick.Layouts 1.13
import QtQuick.Controls 2.13 as QQC2
import org.kde.kirigami 2.13 as Kirigami
import "qrc:/qml/previewwindowcontentscomponents"

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
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.Auto
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
