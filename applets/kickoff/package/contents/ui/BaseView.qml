/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012 Marco Martin <mart@kde.org>
    Copyright (C) 2015-2018  Eike Hein <hein@kde.org>

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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.draganddrop 2.0


Item {
    property alias model: baseView.model
    property alias delegate: baseView.delegate

    property ListView listView: baseView.listView

    function decrementCurrentIndex() {
        baseView.decrementCurrentIndex();
    }

    function incrementCurrentIndex() {
        baseView.incrementCurrentIndex();
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
                baseView.currentIndex = -1;
            }
        }
    }

    KickoffListView {
        id: baseView

        anchors.fill: parent

        currentIndex: -1
        interactive: contentHeight > height
    }
}
