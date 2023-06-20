    /*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.20 as Kirigami

import org.kde.plasma.private.kicker 0.1 as Kicker

PlasmoidItem {
    id: kicker

    anchors.fill: parent

    signal reset

    property bool isDash: plasmoid.pluginName === "org.kde.plasma.kickerdash"

    switchWidth: isDash || !fullRepresentationItem ? 0 :fullRepresentationItem.Layout.minimumWidth
    switchHeight: isDash || !fullRepresentationItem ? 0 :fullRepresentationItem.Layout.minimumHeight

    // this is a bit of a hack to prevent Plasma from spawning a dialog on its own when we're Dash
   preferredRepresentation: isDash ?fullRepresentation : null

   compactRepresentation: isDash ? null : compactRepresentation
   fullRepresentation: isDash ? compactRepresentation : menuRepresentation

    property Component itemListDialogComponent: Qt.createComponent(Qt.resolvedUrl("./ItemListDialog.qml"))
    property Item dragSource: null

    property QtObject globalFavorites: rootModel.favoritesModel
    property QtObject systemFavorites: rootModel.systemFavoritesModel

    Plasmoid.icon: plasmoid.configuration.useCustomButtonImage ? plasmoid.configuration.customButtonImage : plasmoid.configuration.icon

    onSystemFavoritesChanged: {
        systemFavorites.favorites = plasmoid.configuration.favoriteSystemActions;
    }

    function action_menuedit() {
        processRunner.runMenuEditor();
    }

    function updateSvgMetrics() {
        lineSvg.horLineHeight = lineSvg.elementSize("horizontal-line").height;
        lineSvg.vertLineWidth = lineSvg.elementSize("vertical-line").width;
    }

    Component {
        id: compactRepresentation
        CompactRepresentation {}
    }

    Component {
        id: menuRepresentation
        MenuRepresentation {}
    }

    Kicker.RootModel {
        id: rootModel

        autoPopulate: false

        appNameFormat: plasmoid.configuration.appNameFormat
        flat: kicker.isDash || plasmoid.configuration.limitDepth
        sorted: plasmoid.configuration.alphaSort
        showSeparators: !kicker.isDash
        // TODO: appletInterface property now can be ported to "applet" and have the real Applet* assigned directly
        appletInterface: kicker

        showAllApps: kicker.isDash
        showAllAppsCategorized: true
        showTopLevelItems: !kicker.isDash
        showRecentApps: plasmoid.configuration.showRecentApps
        showRecentDocs: plasmoid.configuration.showRecentDocs
        recentOrdering: plasmoid.configuration.recentOrdering

        onShowRecentAppsChanged: {
            plasmoid.configuration.showRecentApps = showRecentApps;
        }

        onShowRecentDocsChanged: {
            plasmoid.configuration.showRecentDocs = showRecentDocs;
        }

        onRecentOrderingChanged: {
            plasmoid.configuration.recentOrdering = recentOrdering;
        }

        Component.onCompleted: {
            favoritesModel.initForClient("org.kde.plasma.kicker.favorites.instance-" + plasmoid.id)

            if (!plasmoid.configuration.favoritesPortedToKAstats) {
                if (favoritesModel.count < 1) {
                    favoritesModel.portOldFavorites(plasmoid.configuration.favoriteApps);
                }
                plasmoid.configuration.favoritesPortedToKAstats = true;
            }
        }
    }

    Connections {
        target: globalFavorites

        function onFavoritesChanged() {
            plasmoid.configuration.favoriteApps = target.favorites;
        }
    }

    Connections {
        target: systemFavorites

        function onFavoritesChanged() {
            plasmoid.configuration.favoriteSystemActions = target.favorites;
        }
    }

    Connections {
        target: plasmoid.configuration

        function onFavoriteAppsChanged() {
            globalFavorites.favorites = plasmoid.configuration.favoriteApps;
        }

        function onFavoriteSystemActionsChanged() {
            systemFavorites.favorites = plasmoid.configuration.favoriteSystemActions;
        }
    }

    Kicker.RunnerModel {
        id: runnerModel

        appletInterface: plasmoid

        favoritesModel: globalFavorites

        runners: {
            const results = ["krunner_services", "krunner_systemsettings"];

            if (kicker.isDash) {
                results.push("krunner_sessions", "krunner_powerdevil", "calculator", "unitconverter");
            }

            if (plasmoid.configuration.useExtraRunners) {
                results.push(...plasmoid.configuration.extraRunners);
            }

            return results;
        }

        deleteWhenEmpty: true
    }

    Kicker.DragHelper {
        id: dragHelper

        dragIconSize: Kirigami.Units.iconSizes.medium
    }

    Kicker.ProcessRunner {
        id: processRunner
    }

    Kicker.WindowSystem {
        id: windowSystem
    }

    KSvg.FrameSvgItem {
        id: highlightItemSvg

        visible: false

        imagePath: "widgets/viewitem"
        prefix: "hover"
    }

    KSvg.FrameSvgItem {
        id: listItemSvg

        visible: false

        imagePath: "widgets/listitem"
        prefix: "normal"
    }

    KSvg.Svg {
        id: arrows

        imagePath: "widgets/arrows"
        size: "16x16"
    }

    KSvg.Svg {
        id: lineSvg
        imagePath: "widgets/line"

        property int horLineHeight
        property int vertLineWidth
    }

    PlasmaComponents3.Label {
        id: toolTipDelegate

        width: contentWidth
        height: undefined

        property Item toolTip

        text: toolTip ? toolTip.text : ""
    }

    Timer {
        id: justOpenedTimer

        repeat: false
        interval: 600
    }

    Connections {
        target: kicker

        function onExpandedChanged(expanded) {
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
        if (plasmoid.hasOwnProperty("activationTogglesExpanded")) {
            plasmoid.activationTogglesExpanded = !kicker.isDash
        }

        windowSystem.focusIn.connect(enableHideOnWindowDeactivate);
        kicker.hideOnWindowDeactivate = true;

        if (plasmoid.immutability !== PlasmaCore.Types.SystemImmutable) {
            plasmoid.setAction("menuedit", i18n("Edit Applications…"), "kmenuedit");
        }

        updateSvgMetrics();
        PlasmaCore.Theme.themeChanged.connect(updateSvgMetrics);

        rootModel.refreshed.connect(reset);

        dragHelper.dropped.connect(resetDragSource);
    }
}
