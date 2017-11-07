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
import org.kde.kcm 1.0

Kirigami.Page {
    id: root

    title: kcm.name
    implicitWidth: Kirigami.Units.gridUnit * 20
    implicitHeight: Kirigami.Units.gridUnit * 20

    property alias view: view

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: footer ? Kirigami.Units.smallSpacing : 0

    QtControls.ScrollView {
        id: scroll
        anchors {
            top: parent.top
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
        width: Math.floor((root.width - Kirigami.Units.gridUnit*2) / view.cellWidth) * view.cellWidth + Kirigami.Units.gridUnit*2
        activeFocusOnTab: false

        GridView {
            id: view
            anchors {
                fill: parent
                margins: scroll.background ? 2 : 0
                rightMargin: contentHeight > height ? Kirigami.Units.gridUnit : 2
            }
            contentItem.x: Math.round((width - Math.floor(width / cellWidth) * cellWidth) / 2)
            clip: true
            activeFocusOnTab: true
            cellWidth: Kirigami.Units.gridUnit * 10
            cellHeight: cellWidth / 1.6 + Kirigami.Units.gridUnit*2
            keyNavigationEnabled: true
            keyNavigationWraps: true
            highlightMoveDuration: 0
        }
        //not all styles have background defined
        Component.onCompleted: {
            background.visible = true;
        }
    }
}
