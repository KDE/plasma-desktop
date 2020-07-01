/* SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.12 as Kirigami
import org.kde.newstuff 1.0 as KNS

// import org.kde.kcolorschemeeditor 1.0

Kirigami.ApplicationWindow {
    id: root
    minimumWidth: Kirigami.Units.gridUnit * 40
    minimumHeight: Kirigami.Units.gridUnit * 30
    width: Kirigami.Units.gridUnit * 64
    height: Kirigami.Units.gridUnit * 48
    // TODO: use real colorscheme name
    title: "Breeze Dark"

//     pageStack.columnView.columnResizeMode: Kirigami.ColumnView.DynamicColumns
    pageStack.defaultColumnWidth: Math.round(root.width / 4)//pageStack.firstVisibleItem.implicitWidth
    pageStack.globalToolBar.toolbarActionAlignment: Qt.AlignLeft
    pageStack.initialPage: [colorListComponent, previewAreaComponent]
    
    Component {
        id: colorListComponent
        ColorList {
            id: colorList
        }
    }
    Component {
        id: previewAreaComponent
        PreviewArea {
            id: previewArea
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
