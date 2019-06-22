/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
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

import org.kde.plasma.private.kicker 0.1 as Kicker

Item {
    id: searchViewContainer

    anchors.fill: parent

    objectName: "SearchView"

    function decrementCurrentIndex() {
        searchView.decrementCurrentIndex();
    }

    function incrementCurrentIndex() {
        searchView.incrementCurrentIndex();
    }

    function activateCurrentIndex() {
        searchView.currentItem.activate();
    }

    function openContextMenu() {
        searchView.currentItem.openActionMenu();
    }

    Kicker.RunnerModel {
        id: runnerModel

        appletInterface: plasmoid

        runners: {
            var runners = ["desktopsessions", "services", "places",
                           "PowerDevil", "calculator", "unitconverter"];

            if (plasmoid.configuration.useExtraRunners) {
                runners = runners.concat(plasmoid.configuration.runners);
            }
            return runners;
        }
        mergeResults: true

        favoritesModel: globalFavorites
    }

    Connections {
        target: header

        onQueryChanged: {
            runnerModel.query = header.query;
            searchView.currentIndex = 0;

            if (!header.query) {
                searchView.model = null;
            }
        }
    }

    Connections {
        target: runnerModel

        onCountChanged: {
            if (runnerModel.count && !searchView.model) {
                searchView.model = runnerModel.modelForRow(0);
            }
        }
    }

    KickoffListView {
        id: searchView

        anchors.fill: parent

        showAppsByName: false //krunner results have the most relevant field in the "display" column, which is showAppsByName = false
    }
}
