/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.components 3.0 as PC3

KickoffGridView {
    id: root

    view.cellWidth: Math.round(KickoffSingleton.listDelegateContentHeight * 2.5)
    view.cellHeight: view.cellWidth

    /**
     * Request hiding the section view
     */
    signal hideSectionViewRequested(int index)

    property string currentSection

    delegate: PC3.AbstractButton {
        id: sectionButton

        width: view.cellWidth
        height: view.cellHeight

        hoverEnabled: true
        onHoveredChanged: if (hovered) {
            root.view.currentIndex = index;
        }

        contentItem: PC3.Label {
            anchors {
                fill: parent
                bottomMargin: KickoffSingleton.fontMetrics.descent
            }
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            maximumLineCount: 1
            elide: Text.ElideRight
            font.pixelSize: KickoffSingleton.listDelegateContentHeight
            text: modelData["section"]
        }

        onClicked: root.hideSectionViewRequested(modelData["firstIndex"])
    }

    Component.onCompleted: {
        for (let i = 0; i < model.length; i++) {
            if (model[i]["section"] === root.currentSection) {
                view.positionViewAtIndex(i, ListView.Beginning);
                view.currentIndex = i;
                return;
            }
        }
    }
}
