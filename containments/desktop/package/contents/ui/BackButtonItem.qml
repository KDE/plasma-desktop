/*
    SPDX-FileCopyrightText: 2011-2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaCore.FrameSvgItem {
    id: upButton

    width: gridView.cellWidth
    height: visible ? gridView.cellHeight : 0

    visible: history.length !== 0

    property bool ignoreClick: false
    property bool containsDrag: false

    imagePath: "widgets/viewitem"

    function handleDragMove() {
        containsDrag = true;
        hoverActivateTimer.restart();
    }

    function endDragMove() {
        containsDrag = false;
        hoverActivateTimer.stop();
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        acceptedButtons: Qt.LeftButton | Qt.BackButton
        hoverEnabled: true

        onContainsMouseChanged: {
            gridView.hoveredItem = null;
        }

        onPressed: {
            if (mouse.buttons & Qt.BackButton) {
                if (root.isPopup && dir.resolvedUrl !== dir.resolve(Plasmoid.configuration.url)) {
                    doBack();
                    ignoreClick = true;
                }
            }
        }

        onClicked: {
            if (ignoreClick) {
                ignoreClick = false;
                return;
            }

            doBack();
        }
    }

    PlasmaCore.IconItem {
        id: icon

        anchors {
            left: parent.left
            leftMargin: PlasmaCore.Units.smallSpacing
            verticalCenter: parent.verticalCenter
        }

        width: gridView.iconSize
        height: gridView.iconSize

        source: "arrow-left"
    }

    PlasmaComponents.Label {
        id: label

        anchors {
            left: icon.right
            leftMargin: PlasmaCore.Units.smallSpacing * 2
            verticalCenter: parent.verticalCenter
        }

        width:  parent.width - icon.width - (PlasmaCore.Units.smallSpacing * 4);

        height: undefined // Unset PlasmaComponents.Label's default.

        textFormat: Text.PlainText

        maximumLineCount: root.isPopup ? 1 : Plasmoid.configuration.textLines
        wrapMode: Text.Wrap
        elide: Text.ElideRight

        text: i18n("Back")
    }

    Timer {
        id: hoverActivateTimer

        interval: root.hoverActivateDelay

        onTriggered: doBack()
    }

    states: [
        State {
            name: "hover"
            when: mouseArea.containsMouse || containsDrag

            PropertyChanges {
                target: upButton
                prefix: "hover"
            }
        }
    ]
}
