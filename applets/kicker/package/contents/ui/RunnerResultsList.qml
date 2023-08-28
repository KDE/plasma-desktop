/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.plasmoid 2.0

FocusScope {
    id: runnerMatchesScope
    width: runnerMatches.width + vertLine.width + vertLine.anchors.leftMargin + resultColumn.anchors.leftMargin

    readonly property alias listView: runnerMatches.listView
    property alias currentIndex: runnerMatches.currentIndex
    property alias count: runnerMatches.count

    Accessible.name: header.text
    Accessible.role: Accessible.MenuItem
    Keys.forwardTo: runnerMatches

    KSvg.SvgItem {
        id: vertLine

        anchors.left: parent.left
        anchors.leftMargin: (index > 0 ) ? Kirigami.Units.smallSpacing : 0

        width: (index > 0 ) ? lineSvg.vertLineWidth : 0
        height: parent.height

        visible: (index > 0)

        svg: lineSvg
        elementId: "vertical-line"
    }

    Column {
        id: resultColumn
        anchors {
            top: Plasmoid.configuration.alignResultsToBottom ? undefined : parent.top
            bottom: Plasmoid.configuration.alignResultsToBottom ? parent.bottom : undefined
            left: vertLine.right
            leftMargin: vertLine.visible ? spacing : 0
        }
        spacing: Kirigami.Units.smallSpacing

        PlasmaComponents3.Label {
            id: header

            width: runnerMatches.width
            height: runnerMatches.itemHeight

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignTop

            textFormat: Text.PlainText
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
            font.weight: Font.Bold

            text: (runnerMatches.model !== null) ? runnerMatches.model.name : ""
        }

        ItemListView {
            id: runnerMatches

            height: Math.min(runnerMatchesScope.height - header.height - spacing, (model?.count ?? 0) * itemHeight)

            iconsEnabled: true

            model: runnerModel.modelForRow(index)

            KeyNavigation.up: runnerMatchesScope.KeyNavigation.up
            KeyNavigation.down: runnerMatchesScope.KeyNavigation.down
            Keys.onLeftPressed: event => runnerMatchesScope.Keys.onLeftPressed(event)
            Keys.onRightPressed: event => runnerMatchesScope.Keys.onRightPressed(event)
        }
    }
}
