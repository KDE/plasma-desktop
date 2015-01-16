/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.private.kicker 0.1 as Kicker

Item {
    id: kicker

    anchors.fill: parent

    signal reset

    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation

    Plasmoid.compactRepresentation: CompactRepresentation {}
    Plasmoid.fullRepresentation: FullRepresentation { id: fullRepr }

    property QtObject itemListDialogComponent: Qt.createComponent("ItemListDialog.qml");
    property Item dragSource: null

    function action_menuedit() {
        processRunner.runMenuEditor();
    }

    function updateSvgMetrics() {
        lineSvg.horLineHeight = lineSvg.elementSize("horizontal-line").height;
        lineSvg.vertLineWidth = lineSvg.elementSize("vertical-line").width;
    }

    Kicker.RootModel {
        id: rootModel

        appNameFormat: plasmoid.configuration.appNameFormat
        flat: plasmoid.configuration.limitDepth
        appletInterface: plasmoid

        onRecentAppsModelChanged: {
            recentAppsModel.recentApps = plasmoid.configuration.recentApps;
            runnerModel.appLaunched.connect(recentAppsModel.addApp);
        }

        Component.onCompleted: {
            favoritesModelForPrefix("app").favorites = plasmoid.configuration.favoriteApps;
            favoritesModelForPrefix("sys").favorites = plasmoid.configuration.favoriteSystemActions;
            recentAppsModel.recentApps = plasmoid.configuration.recentApps;
            runnerModel.appLaunched.connect(recentAppsModel.addApp);
        }
    }

    Connections {
        target: plasmoid.configuration

        onFavoriteAppsChanged: {
            rootModel.favoritesModelForPrefix("app").favorites = plasmoid.configuration.favoriteApps;
        }

        onFavoriteSystemActionsChanged: {
            rootModel.favoritesModelForPrefix("sys").favorites = plasmoid.configuration.favoriteSystemActions;
        }

        onRecentAppsChanged: {
            rootModel.recentAppsModel.recentApps = plasmoid.configuration.recentApps;
        }
    }

    Connections {
        target: rootModel.favoritesModelForPrefix("app")

        onFavoritesChanged: {
            plasmoid.configuration.favoriteApps = target.favorites;
        }
    }

    Connections {
        target: rootModel.favoritesModelForPrefix("sys")

        onFavoritesChanged: {
            plasmoid.configuration.favoriteSystemActions = target.favorites;
        }
    }

    Connections {
        target: rootModel.recentAppsModel

        onRecentAppsChanged: {
            plasmoid.configuration.recentApps = target.recentApps;
        }
    }

    Kicker.RunnerModel {
        id: runnerModel

        runners: plasmoid.configuration.useExtraRunners ? new Array("services").concat(plasmoid.configuration.extraRunners) : "services"
    }

    Kicker.DragHelper {
        id: dragHelper
    }

    Kicker.ProcessRunner {
        id: processRunner;
    }

    Kicker.WindowSystem {
        id: windowSystem;
    }

    PlasmaCore.FrameSvgItem {
        id : highlightItemSvg

        visible: false

        imagePath: "widgets/viewitem"
        prefix: "hover"
    }

    PlasmaCore.FrameSvgItem {
        id : listItemSvg

        visible: false

        imagePath: "widgets/listitem"
        prefix: "normal"
    }

    PlasmaCore.Svg {
        id: arrows

        imagePath: "widgets/arrows"
        size: "16x16"
    }

    PlasmaCore.Svg {
        id: lineSvg
        imagePath: "widgets/line"

        property int horLineHeight
        property int vertLineWidth
    }

    PlasmaComponents.Label {
        id: toolTipDelegate

        width: contentWidth
        height: contentHeight

        property Item toolTip

        text: (toolTip != null) ? toolTip.text : ""
    }

    Timer {
        id: justOpenedTimer

        repeat: false
        interval: 600
    }

    Connections {
        target: plasmoid

        onExpandedChanged: {
            if (expanded) {
                windowSystem.monitorWindowVisibility(plasmoid.fullRepresentationItem);
                justOpenedTimer.start();
            } else {
                kicker.reset();
            }
        }
    }

    function resetDragSource() {
        dragSource = null;
    }

    function enableHideOnWindowDeactivate() {
        plasmoid.hideOnWindowDeactivate = true;
    }

    Component.onCompleted: {
        windowSystem.focusOut.connect(enableHideOnWindowDeactivate);
        plasmoid.hideOnWindowDeactivate = true;

        plasmoid.setAction("menuedit", i18n("Edit Applications..."));

        updateSvgMetrics();
        theme.themeChanged.connect(updateSvgMetrics);

        rootModel.refreshing.connect(reset);

        dragHelper.dropped.connect(resetDragSource);
    }
}
