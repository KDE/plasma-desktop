/*
   Copyright (c) 2017 Marco Martin <mart@kde.org>

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


Kirigami.Page {
    implicitWidth: Kirigami.Units.gridUnit * 20
    implicitHeight: Kirigami.Units.gridUnit * 20

    property alias view: view
    leftPadding: 0
    topPadding: 0
    rightPadding: 0
    bottomPadding: 0

    ColumnLayout {
        anchors.fill:parent
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
                highlightMoveDuration: 0
                onCurrentIndexChanged: {
                    kcm.selectedThemeRow = currentIndex;
                    positionViewAtIndex(currentIndex, ListView.Beginning);
                }
            }
        }
        Kirigami.Separator {
            Layout.fillWidth: true
            Layout.maximumHeight: 1
            visible: !view.atYEnd
        }
    }
}
