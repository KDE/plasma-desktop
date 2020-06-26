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
    // TODO: use real colorscheme name
    title: "Breeze Dark"
    width: Math.round(height / 0.75)
    height: Kirigami.Units.gridUnit * 40

    pageStack.globalToolBar.toolbarActionAlignment: Qt.AlignLeft
    pageStack.initialPage: [colorListComponent, previewAreaComponent]
    
    Component {
        id: colorListComponent
        ColorList {
            id: colorList
            Layout.fillHeight: true
            Layout.minimumWidth: this.implicitWidth
            Layout.preferredWidth: root.width * 0.25
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
