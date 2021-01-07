/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012 Marco Martin <mart@kde.org>
    Copyright (C) 2015-2018  Eike Hein <hein@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
