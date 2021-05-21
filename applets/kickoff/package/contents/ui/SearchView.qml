/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2015-2018 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
