/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
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

import org.kde.plasma.private.kicker 0.1 as Kicker

FocusScope {
    id: searchViewContainer

    objectName: "SearchView"

    function keyNavUp() {
        return searchView.keyNavUp();
    }

    function keyNavDown() {
        return searchView.keyNavDown();
    }

    function activateCurrentIndex() {
        searchView.currentItem.activate();
    }

    function openContextMenu() {
        searchView.currentItem.openActionMenu();
    }

    property ListView listView: searchView.listView

    Kicker.RunnerModel {
        id: runnerModel
        appletInterface: plasmoid
        mergeResults: true
        favoritesModel: globalFavorites
    }

    Connections {
        target: header

        function onQueryChanged() {
            runnerModel.query = header.query;

            if (!header.query) {
                searchView.model = null;
            }
        }
    }

    Connections {
        target: runnerModel

        function onCountChanged() {
            if (runnerModel.count && !searchView.model) {
                searchView.model = runnerModel.modelForRow(0);
            }
        }
    }

    // if search loaded that means that we have a query
    Component.onCompleted: {
        runnerModel.query = header.query;
    }

    KickoffListView {
        id: searchView

        focus: true

        // we specifically only reverse search results (improves UX substantially)
        upsideDown: mainTabGroup.state == "top"

        // set index to 0 when model *loads*
        onModelChanged: {
            if (model) {
                searchView.currentIndex = 0
            }
        }

        anchors.fill: parent
    }
}
