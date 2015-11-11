/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright (C) 2012  Marco Martin <mart@kde.org>
    Copyright (C) 2013  David Edmundson <davidedmundson@kde.org>
    Copyright (C) 2015  Eike Hein <hein@kde.org>

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
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

import org.kde.plasma.private.kicker 0.1 as Kicker

Item {

    Plasmoid.switchWidth: units.gridUnit * 20
    Plasmoid.switchHeight: units.gridUnit * 30

    Plasmoid.fullRepresentation: FullRepresentation {}

    Plasmoid.icon: plasmoid.configuration.icon

    property QtObject globalFavorites: rootModelFavorites

    Kicker.AppsModel {
        id: rootModel

        appletInterface: plasmoid

        appNameFormat: plasmoid.configuration.showAppsByName ? 0 : 1
        flat: false
        showSeparators: false

        favoritesModel: Kicker.FavoritesModel {
            id: rootModelFavorites

            Component.onCompleted: {
                favorites = plasmoid.configuration.favorites;
            }
        }
    }

    Connections {
        target: globalFavorites

        onFavoritesChanged: {
            plasmoid.configuration.favorites = target.favorites;
        }
    }

    Connections {
        target: plasmoid.configuration

        onFavoriteAppsChanged: {
            globalFavorites.favorites = plasmoid.configuration.favorites;
        }
    }

    Kicker.RunnerModel {
        id: runnerModel

        runners: plasmoid.configuration.runners
        mergeResults: true

        favoritesModel: globalFavorites
    }

    Kicker.DragHelper {
        id: dragHelper
    }

    Kicker.ProcessRunner {
        id: processRunner;
    }

    function action_menuedit() {
        processRunner.runMenuEditor();
    }

    Component.onCompleted: {
        plasmoid.setAction("menuedit", i18n("Edit Applications..."));
    }
} // root
