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
import org.kde.kirigami 2.3 as Kirigami

QtControls.ScrollView {
    id: scroll
    property alias view: view
    property int cellWidth: Math.min(Math.floor((parent.width - scrollBarSpace - 4) / 2), Kirigami.Units.gridUnit * 10)
    property int cellHeight: cellWidth / 1.6 + Kirigami.Units.gridUnit

    readonly property int scrollBarSpace: Kirigami.Units.gridUnit

    implicitWidth: Math.max( Math.min(parent.width, cellWidth * 2 + scroll.scrollBarSpace)
                , Math.floor(view.availableWidth / cellWidth) * cellWidth + scroll.scrollBarSpace + 4)
    activeFocusOnTab: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    Component.onCompleted: scroll.background.visible = true;

    GridView {
        id: view
        property int availableWidth: scroll.parent.width - scroll.scrollBarSpace - 4
        anchors {
            fill: parent
            margins: 2
            leftMargin: contentHeight <= height ? scroll.scrollBarSpace/2 : 2
            rightMargin: contentHeight <= height ? scroll.scrollBarSpace/2 : scroll.scrollBarSpace + 2
        }
        clip: true
        activeFocusOnTab: true
        cellWidth: scroll.cellWidth
        cellHeight: scroll.cellHeight
        keyNavigationEnabled: true
        keyNavigationWraps: true
        highlightMoveDuration: 0
    }
}
