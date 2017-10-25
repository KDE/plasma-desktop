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
import QtQuick.Controls 2.2 as QtControls
import org.kde.kirigami 2.2 as Kirigami
import org.kde.kcm 1.0

import org.kde.private.kcm_cursortheme 1.0

ColumnLayout {
    implicitWidth: Kirigami.Units.gridUnit * 20
    implicitHeight: Kirigami.Units.gridUnit * 20

    ConfigModule.quickHelp: i18n("This module lets you configure the mouse cursor theme used.")

    spacing: Kirigami.Units.smallSpacing

    Connections {
        target: kcm
        onSelectedThemeRowChanged: view.currentIndex = kcm.selectedThemeRow;
    }

    ColumnLayout {
        spacing: 0
        Kirigami.Separator {
            Layout.fillWidth: true
            Layout.maximumHeight: 1
            visible: !view.atYBeginning
        }
        QtControls.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            GridView {
                id: view
                cellWidth: Math.floor(width / Math.max(Math.floor(width / (Kirigami.Units.gridUnit*12)), 2)) - Kirigami.Units.gridUnit
                cellHeight: cellWidth / 1.6 + Kirigami.Units.gridUnit*2
                keyNavigationEnabled: true
                keyNavigationWraps: true
                model: kcm.cursorsModel
                highlightMoveDuration: 0
                onCurrentIndexChanged: {
                    kcm.selectedThemeRow = currentIndex;
                    positionViewAtIndex(currentIndex, ListView.Beginning);
                }
                delegate: Delegate {}
            }
        }
        Kirigami.Separator {
            Layout.fillWidth: true
            Layout.maximumHeight: 1
            visible: !view.atYEnd
        }
    }

    RowLayout {
        Layout.maximumHeight: Layout.minimumHeight
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        QtControls.Button {
           // iconName: "document-import"
            text: i18n("&Install From File...")
            onClicked: kcm.installClicked();
            enabled: kcm.canInstall
        }
        QtControls.Button {
           // iconName: "get-hot-new-stuff"
            text: i18n("&Get New Theme...")
            onClicked: kcm.getNewClicked();
            enabled: kcm.canInstall
        }
    }
}

