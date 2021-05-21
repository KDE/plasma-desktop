/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015-2018 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0

FocusScope {
    property alias model: baseView.model
    property alias delegate: baseView.delegate
    property alias contentHeight: baseView.contentHeight
    property alias isExternalManagerMode: baseView.isExternalManagerMode

    property ListView listView: baseView.listView

    function keyNavUp() {
        return baseView.keyNavUp();
    }

    function keyNavDown() {
        return baseView.keyNavDown();
    }

    function activateCurrentIndex() {
        baseView.currentItem.activate();
    }

    function openContextMenu() {
        baseView.currentItem.openActionMenu();
    }

    Connections {
        target: plasmoid

        function onExpandedChanged() {
            if (!plasmoid.expanded) {
                baseView.currentIndex = 0;
            }
        }
    }

    focus: true

    KickoffListView {
        id: baseView

        focus: true
        anchors.fill: parent

        interactive: contentHeight > height
    }
}
