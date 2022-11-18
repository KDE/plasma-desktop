/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtGraphicalEffects 1.15
// Deliberately imported after QtQuick to avoid missing restoreMode property in Binding. Fix in Qt 6.
import QtQml 2.15

import org.kde.kquickcontrolsaddons 2.0
import org.kde.kwindowsystem 1.0
import org.kde.milou 0.3 as Milou
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.private.shell 2.0

import org.kde.plasma.private.kicker 0.1 as Kicker

import "code/tools.js" as Tools

/* TODO
 * Reverse favoritesRow layout + keyboard nav + filter list text alignment in rtl locales.
 * Keep cursor column when arrow'ing down past non-full trailing rows into a lower grid.
 * Make DND transitions cleaner by performing an item swap instead of index reinsertion.
*/

Kicker.DashboardWindow {
    id: root

    property bool smallScreen: ((Math.floor(width / PlasmaCore.Units.iconSizes.huge) <= 22) || (Math.floor(height / PlasmaCore.Units.iconSizes.huge) <= 14))

    property int iconSize: smallScreen ? PlasmaCore.Units.iconSizes.large : PlasmaCore.Units.iconSizes.large
    property int cellSize: iconSize + (2 * PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height)
        + (2 * PlasmaCore.Units.smallSpacing)
        + (2 * Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
                        highlightItemSvg.margins.left + highlightItemSvg.margins.right))
    property int columns: Math.floor(((smallScreen ? 85 : 80)/100) * Math.ceil(width / cellSize))
    property bool searching: searchField.text !== ""
    property var widgetExplorer: null

    keyEventProxy: searchField

    backgroundColor: Qt.rgba(PlasmaCore.Theme.backgroundColor.r, PlasmaCore.Theme.backgroundColor.g, PlasmaCore.Theme.backgroundColor.b, 0.75)

    onKeyEscapePressed: {
        if (searching) {
            searchField.clear();
        } else {
            root.toggle();
        }
    }

    onVisibleChanged: {
        tabBar.activeTab = 0;
        reset();

        if (visible) {
            preloadAllAppsTimer.restart();
        }
    }

    onSearchingChanged: {
        if (!searching) {
            reset();
        } else {
            filterList.currentIndex = -1;

            if (tabBar.activeTab === 1) {
                widgetExplorer.widgetsModel.filterQuery = "";
                widgetExplorer.widgetsModel.filterType = "";
            }
        }
    }

    function reset() {
        searchField.clear();
        searchField.forceActiveFocus();
        globalFavoritesGrid.currentIndex = -1;
        systemFavoritesGrid.currentIndex = -1;
        filterList.currentIndex = 0;
        funnelModel.sourceModel = rootModel.modelForRow(0);
        mainGrid.model = (tabBar.activeTab === 0) ? funnelModel : root.widgetExplorer.widgetsModel;
        mainGrid.currentIndex = -1;
        filterListScrollArea.focus = true;
        filterList.model = (tabBar.activeTab === 0) ? rootModel : root.widgetExplorer.filterModel;
    }

    function updateWidgetExplorer() {
        if (tabBar.activeTab === 1 /* Widgets */ || tabBar.hoveredTab === 1) {
            if (!root.widgetExplorer) {
                root.widgetExplorer = widgetExplorerComponent.createObject(root, {
                    containment: containmentInterface.screenContainment(plasmoid)
                });
            }
        } else if (root.widgetExplorer) {
            root.widgetExplorer.destroy();
            root.widgetExplorer = null;
        }
    }

    mainItem: MouseArea {
        id: rootItem

        anchors.fill: parent

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        Keys.onDownPressed: searchResults.incrementCurrentIndex()
        Keys.onUpPressed: searchResults.decrementCurrentIndex()
        Keys.onReturnPressed: searchResults.runCurrentIndex(event)
        Keys.onEnterPressed: searchResults.runCurrentIndex(event)

        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        Connections {
            target: kicker

            function onReset() {
                if (!root.searching) {
                    filterList.applyFilter();

                    if (tabBar.activeTab === 0) {
                        funnelModel.reset();
                    }
                }
            }

            function onDragSourceChanged() {
                if (!kicker.dragSource) {
                    // FIXME TODO HACK: Reset all views post-DND to work around
                    // mouse grab bug despite QQuickWindow::mouseGrabberItem==0x0.
                    // Needs a more involved hunt through Qt Quick sources later since
                    // it's not happening with near-identical code in the menu repr.
                    rootModel.refresh();
                } else if (tabBar.activeTab === 1) {
                    root.toggle();
                    kwindowsystem.showingDesktop = true;
                }
            }
        }

        KWindowSystem {
            id: kwindowsystem
        }

        Component {
            id: widgetExplorerComponent

            WidgetExplorer { showSpecialFilters: false }
        }

        Connections {
            target: plasmoid
            function onUserConfiguringChanged() {
                if (plasmoid.userConfiguring) {
                    root.hide()
                }
            }
        }

        PlasmaComponents.Menu {
            id: contextMenu

            PlasmaComponents.MenuItem {
                action: plasmoid.action("configure")
            }
        }

        PlasmaExtras.Heading {
            id: dummyHeading

            visible: false

            width: 0

            level: 1
        }

        TextMetrics {
            id: headingMetrics

            font: dummyHeading.font
        }

        Kicker.FunnelModel {
            id: funnelModel

            onSourceModelChanged: {
                if (mainColumn.visible) {
                    mainGrid.currentIndex = -1;
                    mainGrid.forceLayout();
                }
            }
        }

        Timer {
            id: preloadAllAppsTimer

            property bool done: false

            interval: 1000
            repeat: false

            onTriggered: {
                if (done || root.searching) {
                    return;
                }

                for (var i = 0; i < rootModel.count; ++i) {
                    var model = rootModel.modelForRow(i);

                    if (model.description === "KICKER_ALL_MODEL") {
                        allAppsGrid.model = model;
                        done = true;
                        break;
                    }
                }
            }

            function defer() {
                if (running && !done) {
                    restart();
                }
            }
        }

        Kicker.ContainmentInterface {
            id: containmentInterface
        }

        Row {
            id: favoritesRow

            anchors {
                top: parent.top
                topMargin: PlasmaCore.Units.gridUnit * 4
                bottom: parent.verticalCenter
                bottomMargin: (PlasmaCore.Units.gridUnit * 2)
                horizontalCenter: parent.horizontalCenter
            }

            width: (root.columns * root.cellSize) + (2 * spacing)

            spacing: PlasmaCore.Units.gridUnit * 2

            ItemGridView {
                id: globalFavoritesGrid

                anchors {
                    horizontalCenter: parent.horizontalCenter
                }

                property int rows: (Math.floor((parent.height - PlasmaCore.Units.largeSpacing) / root.cellSize))

                width: root.width / 6
                height: rows * root.cellSize

                iconSize: root.iconSize

                model: globalFavorites

                dropEnabled: true
                usesPlasmaTheme: false

                opacity: enabled ? 1.0 : 0.3

                Behavior on opacity { SmoothedAnimation { duration: PlasmaCore.Units.longDuration; velocity: 0.01 } }

                onCurrentIndexChanged: {
                    preloadAllAppsTimer.defer();
                }

                // onKeyNavRight: tryActivate(currentRow(), 0)

                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Tab) {
                        event.accepted = true;

                        if (tabBar.visible) {
                            tabBar.focus = true;
                        } else if (root.searching) {
                            cancelSearchButton.focus = true;
                        } else {
                            mainColumn.tryActivate(0, 0);
                        }
                    } else if (event.key === Qt.Key_Backtab) {
                        event.accepted = true;
                        systemFavoritesGrid.tryActivate(0, 0);
                    }
                }

                Binding {
                    target: globalFavorites
                    property: "iconSize"
                    value: root.iconSize
                    restoreMode: Binding.RestoreBinding
                }
            }
        }

        Row {
            id: searchFieldRow

            anchors {
                top: favoritesRow.bottom
                topMargin: PlasmaCore.Units.gridUnit * - 15
                bottom: parent.bottom
                bottomMargin: (PlasmaCore.Units.gridUnit * 2)
                horizontalCenter: parent.horizontalCenter
            }

            PlasmaComponents.TextField {
                id: searchField

                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                width: searchResults.width / 2

                color: PlasmaCore.Theme.textColor
                selectionColor: PlasmaCore.Theme.highlightColor

                font.pointSize: 10
                wrapMode: Text.NoWrap
                persistentSelection: true

                placeholderText: i18n("Search...")

                function clear() {
                    text = "";
                }

            }
        }

        Milou.ResultsView {
            id: searchResults

            visible: searchField.text

            anchors {
                top: searchFieldRow.bottom
                topMargin: PlasmaCore.Units.gridUnit * - 40
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
            }
            width: parent.width / 2
            height: parent.height - PlasmaCore.Units.largeSpacing

            queryString: searchField.text

            onActivated: {
                searchField.clear();
                searchField.forceActiveFocus();
                root.toggle();
            }
        }

        onPressed: mouse => {
            if (mouse.button === Qt.RightButton) {
                contextMenu.open(mouse.x, mouse.y);
            }
        }

        onClicked: mouse => {
            if (mouse.button === Qt.LeftButton) {
                searchField.clear();
                searchField.forceActiveFocus();
                root.toggle();
            }
        }
    }

    Component.onCompleted: {
        rootModel.refresh();
    }
}
