/***************************************************************************
 *   Copyright (C) 2013-2014 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

FocusScope {
    width: runnerMatches.width + vertLine.width + vertLine.anchors.leftMargin + runnerMatches.anchors.leftMargin
    height: parent.height

    signal keyNavigationAtListEnd

    property alias currentIndex: runnerMatches.currentIndex
    property alias count: runnerMatches.count
    property alias containsMouse: runnerMatches.containsMouse

    Accessible.name: header.text
    Accessible.role: Accessible.MenuItem

    PlasmaCore.SvgItem {
        id: vertLine

        anchors.left: parent.left
        anchors.leftMargin: (index > 0 ) ? units.smallSpacing : 0

        width: (index > 0 ) ? lineSvg.vertLineWidth : 0
        height: parent.height

        visible: (index > 0)

        svg: lineSvg
        elementId: "vertical-line"
    }

    PlasmaComponents.Label {
        id: header

        anchors.left: vertLine.right

        width: runnerMatches.width
        height: runnerMatches.itemHeight

        horizontalAlignment: Text.AlignHCenter

        textFormat: Text.PlainText
        wrapMode: Text.NoWrap
        elide: Text.ElideRight
        font.weight: Font.Bold

        text: (runnerMatches.model !== null) ? runnerMatches.model.name : ""
    }

    ItemListView {
        id: runnerMatches

        anchors.top: plasmoid.configuration.alignResultsToBottom ? undefined : header.bottom
        anchors.bottom: plasmoid.configuration.alignResultsToBottom ? parent.bottom : undefined
        anchors.bottomMargin: (index == 0 && anchors.bottom !== undefined) ? searchField.height + (2 * units.smallSpacing) : undefined
        anchors.left: vertLine.right
        anchors.leftMargin: (index > 0) ? units.smallSpacing : 0

        height: {
            var listHeight = (((index == 0)
                ? rootList.height : runnerColumns.height) - header.height);

            if (model && model.count) {
                return Math.min(listHeight, model.count * itemHeight);
            }

            return listHeight;
        }

        focus: true

        iconsEnabled: true
        keyNavigationWraps: (index != 0)

        resetOnExitDelay: 0

        model: runnerModel.modelForRow(index)

        onModelChanged: {
            if (model === undefined || model === null) {
                enabled: false;
                visible: false;
            }
        }

        onCountChanged: {
            if (index == 0 && searchField.focus) {
                currentIndex = 0;
            }
        }
    }

    Component.onCompleted: {
        runnerMatches.keyNavigationAtListEnd.connect(keyNavigationAtListEnd);
    }
}
