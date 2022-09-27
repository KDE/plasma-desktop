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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.private.shell 2.0

import org.kde.plasma.private.kicker 0.1 as Kicker

import "code/tools.js" as Tools

/* TODO
 * Reverse middleRow layout + keyboard nav + filter list text alignment in rtl locales.
 * Keep cursor column when arrow'ing down past non-full trailing rows into a lower grid.
 * Make DND transitions cleaner by performing an item swap instead of index reinsertion.
*/

Kicker.DashboardWindow {
    id: root

    property bool smallScreen: ((Math.floor(width / PlasmaCore.Units.iconSizes.huge) <= 22) || (Math.floor(height / PlasmaCore.Units.iconSizes.huge) <= 14))

    property int iconSize: smallScreen ? PlasmaCore.Units.iconSizes.large : PlasmaCore.Units.iconSizes.huge
    property int cellSize: iconSize + (2 * PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height)
        + (2 * PlasmaCore.Units.smallSpacing)
        + (2 * Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
                        highlightItemSvg.margins.left + highlightItemSvg.margins.right))
    property int columns: Math.floor(((smallScreen ? 85 : 80)/100) * Math.ceil(width / cellSize))
    property bool searching: searchField.text !== ""
    property var widgetExplorer: null

    keyEventProxy: searchField
    backgroundColor: Qt.rgba(0, 0, 0, 0.737)

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

        DashboardTabBar {
            id: tabBar

            y: 0

            anchors.horizontalCenter: parent.horizontalCenter

            visible: (plasmoid.immutability === PlasmaCore.Types.Mutable)

            onActiveTabChanged: {
                root.updateWidgetExplorer();
                root.reset();
            }

            onHoveredTabChanged: root.updateWidgetExplorer()

            Keys.onDownPressed: {
                mainColumn.tryActivate(0, 0);
            }
        }

        TextEdit {
            id: searchField

            width: 0
            height: 0

            visible: false

            persistentSelection: true

            onTextChanged: {
                if (tabBar.activeTab === 0) {
                    runnerModel.query = searchField.text;
                } else {
                    root.widgetExplorer.widgetsModel.searchTerm = searchField.text;
                }
            }

            function clear() {
                text = "";
            }

            onSelectionStartChanged: Qt.callLater(searchHeading.updateSelection)
            onSelectionEndChanged: Qt.callLater(searchHeading.updateSelection)
        }

        TextEdit {
            id: searchHeading

            anchors {
                horizontalCenter: parent.horizontalCenter
            }

            y: (middleRow.anchors.topMargin / 2) - (root.smallScreen ? (height/10) : 0)

            font.pointSize: dummyHeading.font.pointSize * 1.5
            wrapMode: Text.NoWrap
            opacity: 1.0

            selectByMouse: false
            cursorVisible: false

            color: "white"

            text: root.searching ? i18n("Searching for '%1'", searchField.text) : i18n("Type to search…")

            function updateSelection() {
                if (!searchField.selectedText) {
                    return;
                }

                var delta = text.lastIndexOf(searchField.text, text.length - 2);
                searchHeading.select(searchField.selectionStart + delta, searchField.selectionEnd + delta);
            }
        }

        PlasmaComponents.ToolButton {
            id: cancelSearchButton

            anchors {
                left: searchHeading.right
                leftMargin: PlasmaCore.Units.largeSpacing
                verticalCenter: searchHeading.verticalCenter
            }

            width: PlasmaCore.Units.iconSizes.large
            height: width

            visible: (searchField.text !== "")

            iconName: "edit-clear"
            flat: false

            onClicked: searchField.clear();

            Keys.onPressed: event => {
                if (event.key === Qt.Key_Tab) {
                    event.accepted = true;

                    if (runnerModel.count) {
                        mainColumn.tryActivate(0, 0);
                    } else {
                        systemFavoritesGrid.tryActivate(0, 0);
                    }
                } else if (event.key === Qt.Key_Backtab) {
                    event.accepted = true;

                    if (tabBar.visible) {
                        tabBar.focus = true;
                    } else if (globalFavoritesGrid.enabled) {
                        globalFavoritesGrid.tryActivate(0, 0);
                    } else {
                        systemFavoritesGrid.tryActivate(0, 0);
                    }
                }
            }
        }

        Row {
            id: middleRow

            anchors {
                top: parent.top
                topMargin: PlasmaCore.Units.gridUnit * (smallScreen ? 8 : 10)
                bottom: parent.bottom
                bottomMargin: (PlasmaCore.Units.gridUnit * 2)
                horizontalCenter: parent.horizontalCenter
            }

            width: (root.columns * root.cellSize) + (2 * spacing)

            spacing: PlasmaCore.Units.gridUnit * 2

            Item {
                id: favoritesColumn

                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }

                width: (columns * root.cellSize) + PlasmaCore.Units.gridUnit

                property int columns: 3

                PlasmaExtras.Heading {
                    id: favoritesColumnLabel

                    enabled: tabBar.activeTab === 0

                    anchors {
                        top: parent.top
                    }

                    x: PlasmaCore.Units.smallSpacing
                    width: parent.width - x

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap

                    color: "white"

                    level: 1

                    text: i18n("Favorites")

                    opacity: enabled ? 1.0 : 0.3

                    Behavior on opacity { SmoothedAnimation { duration: PlasmaCore.Units.longDuration; velocity: 0.01 } }
                }

                PlasmaCore.SvgItem {
                    id: favoritesColumnLabelUnderline

                    enabled: (tabBar.activeTab === 0)

                    anchors {
                        top: favoritesColumnLabel.bottom
                    }

                    width: parent.width - PlasmaCore.Units.gridUnit
                    height: lineSvg.horLineHeight

                    svg: lineSvg
                    elementId: "horizontal-line"

                    opacity: enabled ? 1.0 : 0.3

                    Behavior on opacity { SmoothedAnimation { duration: PlasmaCore.Units.longDuration; velocity: 0.01 } }
                }

                ItemGridView {
                    id: globalFavoritesGrid

                    enabled: tabBar.activeTab === 0

                    anchors {
                        top: favoritesColumnLabelUnderline.bottom
                        topMargin: PlasmaCore.Units.largeSpacing
                    }

                    property int rows: (Math.floor((parent.height - favoritesColumnLabel.height
                        - favoritesColumnLabelUnderline.height - PlasmaCore.Units.largeSpacing) / root.cellSize)
                        - systemFavoritesGrid.rows)

                    width: parent.width
                    height: rows * root.cellSize

                    cellWidth: root.cellSize
                    cellHeight: root.cellSize
                    iconSize: root.iconSize

                    model: globalFavorites

                    dropEnabled: true
                    usesPlasmaTheme: false

                    opacity: enabled ? 1.0 : 0.3

                    Behavior on opacity { SmoothedAnimation { duration: PlasmaCore.Units.longDuration; velocity: 0.01 } }

                    onCurrentIndexChanged: {
                        preloadAllAppsTimer.defer();
                    }

                    onKeyNavRight: {
                        mainColumn.tryActivate(currentRow(), 0);
                    }

                    onKeyNavDown: {
                        systemFavoritesGrid.tryActivate(0, currentCol());
                    }

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

                ItemGridView {
                    id: systemFavoritesGrid

                    anchors {
                        top: globalFavoritesGrid.bottom
                    }

                    property int rows: Math.ceil(count / Math.floor(width / root.cellSize))

                    width: parent.width
                    height: rows * root.cellSize

                    cellWidth: root.cellSize
                    cellHeight: root.cellSize
                    iconSize: root.iconSize

                    model: systemFavorites

                    dropEnabled: true
                    usesPlasmaTheme: true

                    onCurrentIndexChanged: {
                        preloadAllAppsTimer.defer();
                    }

                    onKeyNavRight: {
                        mainColumn.tryActivate(globalFavoritesGrid.rows + currentRow(), 0);
                    }

                    onKeyNavUp: {
                        globalFavoritesGrid.tryActivate(globalFavoritesGrid.rows - 1, currentCol());
                    }

                    Keys.onPressed: event => {
                        if (event.key === Qt.Key_Tab) {
                            event.accepted = true;

                            if (globalFavoritesGrid.enabled) {
                                globalFavoritesGrid.tryActivate(0, 0);
                            } else if (tabBar.visible) {
                                tabBar.focus = true;
                            } else if (root.searching && !runnerModel.count) {
                                cancelSearchButton.focus = true;
                            } else {
                                mainColumn.tryActivate(0, 0);
                            }
                        } else if (event.key === Qt.Key_Backtab) {
                            event.accepted = true;

                            if (filterList.enabled) {
                                filterList.forceActiveFocus();
                            } else if (root.searching && !runnerModel.count) {
                                cancelSearchButton.focus = true;
                            } else {
                                mainColumn.tryActivate(0, 0);
                            }
                        }
                    }
                }
            }

            Item {
                id: mainColumn

                anchors.top: parent.top

                width: (columns * root.cellSize) + PlasmaCore.Units.gridUnit
                height: Math.floor(parent.height / root.cellSize) * root.cellSize + mainGridContainer.headerHeight

                property int columns: root.columns - favoritesColumn.columns - filterListColumn.columns
                property Item visibleGrid: mainGrid

                function tryActivate(row, col) {
                    if (visibleGrid) {
                        visibleGrid.tryActivate(row, col);
                    }
                }

                Item {
                    id: mainGridContainer

                    anchors.fill: parent
                    z: (opacity === 1.0) ? 1 : 0

                    visible: opacity !== 0.0

                    property int headerHeight: mainColumnLabel.height + mainColumnLabelUnderline.height + PlasmaCore.Units.largeSpacing

                    opacity: {
                        if (tabBar.activeTab === 0 && root.searching) {
                            return 0.0;
                        }

                        if (filterList.allApps) {
                            return 0.0;
                        }

                        return 1.0;
                    }

                    onOpacityChanged: {
                        if (opacity === 1.0) {
                            mainColumn.visibleGrid = mainGrid;
                        }
                    }

                    PlasmaExtras.Heading {
                        id: mainColumnLabel

                        anchors {
                            top: parent.top
                        }

                        x: PlasmaCore.Units.smallSpacing
                        width: parent.width - x

                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                        opacity: 1.0

                        color: "white"

                        level: 1

                        text: (tabBar.activeTab === 0) ? funnelModel.description : i18n("Widgets")
                    }

                    PlasmaCore.SvgItem {
                        id: mainColumnLabelUnderline

                        visible: mainGrid.count

                        anchors {
                            top: mainColumnLabel.bottom
                        }

                        width: parent.width - PlasmaCore.Units.gridUnit
                        height: lineSvg.horLineHeight

                        svg: lineSvg
                        elementId: "horizontal-line"
                    }

                    ItemGridView {
                        id: mainGrid

                        anchors {
                            top: mainColumnLabelUnderline.bottom
                            topMargin: PlasmaCore.Units.largeSpacing
                        }

                        width: parent.width
                        height: systemFavoritesGrid.y + systemFavoritesGrid.height - mainGridContainer.headerHeight

                        cellWidth: (tabBar.activeTab === 0 ? root.cellSize : root.cellSize * 2)
                        cellHeight: cellWidth
                        iconSize: (tabBar.activeTab === 0 ? root.iconSize : cellWidth - (PlasmaCore.Units.largeSpacing * 2))

                        model: funnelModel

                        onCurrentIndexChanged: {
                            preloadAllAppsTimer.defer();
                        }

                        onKeyNavLeft: {
                            if (tabBar.activeTab === 0) {
                                var row = currentRow();
                                var target = row + 1 > globalFavoritesGrid.rows ? systemFavoritesGrid : globalFavoritesGrid;
                                var targetRow = row + 1 > globalFavoritesGrid.rows ? row - globalFavoritesGrid.rows : row;
                                target.tryActivate(targetRow, favoritesColumn.columns - 1);
                            }
                        }

                        onKeyNavRight: {
                            filterListScrollArea.focus = true;
                        }

                        onKeyNavUp: {
                            if (tabBar.visible) {
                                tabBar.focus = true;
                            }
                        }

                        onItemActivated: {
                            if (tabBar.activeTab === 1) {
                                root.widgetExplorer.addApplet(currentItem.m.pluginName);
                                root.toggle();
                                kwindowsystem.showingDesktop = true;
                            }
                        }
                    }
                }

                ItemMultiGridView {
                    id: allAppsGrid

                    anchors {
                        top: parent.top
                    }

                    z: (opacity === 1.0) ? 1 : 0
                    width: parent.width
                    height: systemFavoritesGrid.y + systemFavoritesGrid.height

                    visible: opacity !== 0.0

                    opacity: filterList.allApps ? 1.0 : 0.0

                    onOpacityChanged: {
                        if (opacity === 1.0) {
                            allAppsGrid.flickableItem.contentY = 0;
                            mainColumn.visibleGrid = allAppsGrid;
                        }
                    }

                    onKeyNavLeft: {
                        var row = 0;

                        for (var i = 0; i < subGridIndex; i++) {
                            row += subGridAt(i).lastRow() + 2; // Header counts as one.
                        }

                        row += subGridAt(subGridIndex).currentRow();

                        var target = row + 1 > globalFavoritesGrid.rows ? systemFavoritesGrid : globalFavoritesGrid;
                        var targetRow = row + 1 > globalFavoritesGrid.rows ? row - globalFavoritesGrid.rows : row;
                        target.tryActivate(targetRow, favoritesColumn.columns - 1);
                    }

                    onKeyNavRight: {
                        filterListScrollArea.focus = true;
                    }
                }

                ItemMultiGridView {
                    id: runnerGrid

                    anchors {
                        top: parent.top
                    }

                    z: (opacity === 1.0) ? 1 : 0
                    width: parent.width
                    height: Math.min(implicitHeight, systemFavoritesGrid.y + systemFavoritesGrid.height)

                    visible: opacity !== 0.0

                    model: runnerModel

                    grabFocus: true

                    opacity: (tabBar.activeTab === 0 && root.searching) ? 1.0 : 0.0

                    onOpacityChanged: {
                        if (opacity === 1.0) {
                            mainColumn.visibleGrid = runnerGrid;
                        }
                    }

                    onKeyNavLeft: {
                        var row = 0;

                        for (var i = 0; i < subGridIndex; i++) {
                            row += subGridAt(i).lastRow() + 2; // Header counts as one.
                        }

                        row += subGridAt(subGridIndex).currentRow();

                        var target = row + 1 > globalFavoritesGrid.rows ? systemFavoritesGrid : globalFavoritesGrid;
                        var targetRow = row + 1 > globalFavoritesGrid.rows ? row - globalFavoritesGrid.rows : row;
                        target.tryActivate(targetRow, favoritesColumn.columns - 1);
                    }
                }

                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Tab) {
                        event.accepted = true;

                        if (filterList.enabled) {
                            filterList.forceActiveFocus();
                        } else {
                            systemFavoritesGrid.tryActivate(0, 0);
                        }
                    } else if (event.key === Qt.Key_Backtab) {
                        event.accepted = true;

                        if (root.searching) {
                            cancelSearchButton.focus = true;
                        } else if (tabBar.visible) {
                            tabBar.focus = true;
                        } else if (globalFavoritesGrid.enabled) {
                            globalFavoritesGrid.tryActivate(0, 0);
                        } else {
                            systemFavoritesGrid.tryActivate(0, 0);
                        }
                    }
                }
            }

            Item {
                id: filterListColumn

                anchors {
                    top: parent.top
                    topMargin: mainColumnLabelUnderline.y + mainColumnLabelUnderline.height + PlasmaCore.Units.largeSpacing
                    bottom: parent.bottom
                }

                width: columns * root.cellSize

                property int columns: 3

                PlasmaExtras.ScrollArea {
                    id: filterListScrollArea

                    x: root.visible ? 0 : PlasmaCore.Units.gridUnit

                    Behavior on x { SmoothedAnimation { duration: PlasmaCore.Units.longDuration; velocity: 0.01 } }

                    width: parent.width
                    height: mainGrid.height

                    enabled: !root.searching

                    property alias currentIndex: filterList.currentIndex

                    opacity: root.visible ? (root.searching ? 0.30 : 1.0) : 0.3

                    Behavior on opacity { SmoothedAnimation { duration: PlasmaCore.Units.longDuration; velocity: 0.01 } }

                    verticalScrollBarPolicy: (opacity === 1.0) ? Qt.ScrollBarAsNeeded : Qt.ScrollBarAlwaysOff

                    onEnabledChanged: {
                        if (!enabled) {
                            filterList.currentIndex = -1;
                        }
                    }

                    onCurrentIndexChanged: {
                        focus = (currentIndex !== -1);
                    }

                    ListView {
                        id: filterList

                        focus: true

                        property bool allApps: false
                        property int eligibleWidth: width
                        property int hItemMargins: Math.max(highlightItemSvg.margins.left + highlightItemSvg.margins.right,
                            listItemSvg.margins.left + listItemSvg.margins.right)

                        model: rootModel

                        boundsBehavior: Flickable.StopAtBounds
                        snapMode: ListView.SnapToItem
                        spacing: 0
                        keyNavigationWraps: true

                        delegate: MouseArea {
                            id: item

                            signal actionTriggered(string actionId, variant actionArgument)
                            signal aboutToShowActionMenu(variant actionMenu)

                            property var m: model
                            property int textWidth: label.contentWidth
                            property int mouseCol
                            property bool hasActionList: ((model.favoriteId !== null)
                                || (("hasActionList" in model) && (model.hasActionList === true)))
                            property Item menu: actionMenu

                            width: ListView.view.width
                            height: Math.ceil((label.paintedHeight
                                + Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
                                listItemSvg.margins.top + listItemSvg.margins.bottom)) / 2) * 2

                            Accessible.role: Accessible.MenuItem
                            Accessible.name: model.display

                            acceptedButtons: Qt.LeftButton | Qt.RightButton

                            hoverEnabled: true

                            onContainsMouseChanged: {
                                if (!containsMouse) {
                                    updateCurrentItemTimer.stop();
                                }
                            }

                            onPositionChanged: mouse => { // Lazy menu implementation.
                                mouseCol = mouse.x;

                                if (justOpenedTimer.running || ListView.view.currentIndex === 0 || index === ListView.view.currentIndex) {
                                    updateCurrentItem();
                                } else if ((index === ListView.view.currentIndex - 1) && mouse.y < (height - 6)
                                    || (index === ListView.view.currentIndex + 1) && mouse.y > 5) {

                                    if (mouse.x > ListView.view.eligibleWidth - 5) {
                                        updateCurrentItem();
                                    }
                                } else if (mouse.x > ListView.view.eligibleWidth) {
                                    updateCurrentItem();
                                }

                                updateCurrentItemTimer.restart();
                            }

                            onPressed: mouse => {
                                if (mouse.buttons & Qt.RightButton) {
                                    if (hasActionList) {
                                        openActionMenu(item, mouse.x, mouse.y);
                                    }
                                }
                            }

                            onClicked: mouse => {
                                if (mouse.button === Qt.LeftButton) {
                                    updateCurrentItem();
                                }
                            }

                            onAboutToShowActionMenu: {
                                var actionList = hasActionList ? model.actionList : [];
                                Tools.fillActionMenu(i18n, actionMenu, actionList, ListView.view.model.favoritesModel, model.favoriteId);
                            }

                            onActionTriggered: {
                                if (Tools.triggerAction(ListView.view.model, model.index, actionId, actionArgument) === true) {
                                    plasmoid.expanded = false;
                                }
                            }

                            function openActionMenu(visualParent, x, y) {
                                aboutToShowActionMenu(actionMenu);
                                actionMenu.visualParent = visualParent;
                                actionMenu.open(x, y);
                            }

                            function updateCurrentItem() {
                                ListView.view.currentIndex = index;
                                ListView.view.eligibleWidth = Math.min(width, mouseCol);
                            }

                            ActionMenu {
                                id: actionMenu

                                onActionClicked: {
                                    actionTriggered(actionId, actionArgument);
                                }
                            }

                            Timer {
                                id: updateCurrentItemTimer

                                interval: 50
                                repeat: false

                                onTriggered: parent.updateCurrentItem()
                            }

                            PlasmaExtras.Heading {
                                id: label

                                anchors {
                                    fill: parent
                                    leftMargin: highlightItemSvg.margins.left
                                    rightMargin: highlightItemSvg.margins.right
                                }

                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                                opacity: 1.0

                                color: "white"

                                level: 1

                                text: model.display
                            }
                        }

                        highlight: PlasmaExtras.Highlight {
                            x: filterList.currentItem ? filterList.currentItem.x : 0
                            y: filterList.currentItem ? filterList.currentItem.y : 0
                            height: filterList.currentItem ? filterList.currentItem.height : 0
                            width: (highlightItemSvg.margins.left
                                + (filterList.currentItem ? filterList.currentItem.textWidth : 0)
                                + highlightItemSvg.margins.right
                                + PlasmaCore.Units.smallSpacing)

                            visible: filterList.currentItem
                            opacity: filterListScrollArea.focus ? 1.0 : 0.7
                        }

                        highlightFollowsCurrentItem: false
                        highlightMoveDuration: 0
                        highlightResizeDuration: 0

                        onCurrentIndexChanged: applyFilter()

                        onCountChanged: {
                            var width = 0;

                            for (var i = 0; i < rootModel.count; ++i) {
                                headingMetrics.text = rootModel.labelForRow(i);

                                if (headingMetrics.width > width) {
                                    width = headingMetrics.width;
                                }
                            }

                            filterListColumn.columns = Math.ceil(width / root.cellSize);
                            filterListScrollArea.width = width + hItemMargins + (PlasmaCore.Units.gridUnit * 2);
                        }

                        function applyFilter() {
                            if (!root.searching && currentIndex >= 0) {
                                if (tabBar.activeTab === 1) {
                                    root.widgetExplorer.widgetsModel.filterQuery = currentItem.m.filterData;
                                    root.widgetExplorer.widgetsModel.filterType = currentItem.m.filterType;

                                    allApps = false;
                                    funnelModel.sourceModel = model;

                                    return;
                                }

                                if (preloadAllAppsTimer.running) {
                                    preloadAllAppsTimer.stop();
                                }

                                var model = rootModel.modelForRow(currentIndex);

                                if (model.description === "KICKER_ALL_MODEL") {
                                    allAppsGrid.model = model;
                                    allApps = true;
                                    funnelModel.sourceModel = null;
                                    preloadAllAppsTimer.done = true;
                                } else {
                                    funnelModel.sourceModel = model;
                                    allApps = false;
                                }
                            } else {
                                funnelModel.sourceModel = null;
                                allApps = false;
                            }
                        }

                        Keys.onPressed: event => {
                            if (event.key === Qt.Key_Left) {
                                event.accepted = true;

                                const currentRow = Math.max(0, Math.ceil(currentItem.y / mainGrid.cellHeight) - 1);
                                mainColumn.tryActivate(currentRow, mainColumn.columns - 1);
                            } else if (event.key === Qt.Key_Tab) {
                                event.accepted = true;
                                systemFavoritesGrid.tryActivate(0, 0);
                            } else if (event.key === Qt.Key_Backtab) {
                                event.accepted = true;
                                mainColumn.tryActivate(0, 0);
                            }
                        }
                    }
                }
            }
        }

        onPressed: mouse => {
            if (mouse.button === Qt.RightButton) {
                contextMenu.open(mouse.x, mouse.y);
            }
        }

        onClicked: mouse => {
            if (mouse.button === Qt.LeftButton) {
                root.toggle();
            }
        }
    }

    Component.onCompleted: {
        rootModel.refresh();
    }
}
