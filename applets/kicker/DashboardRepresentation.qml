/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import org.kde.plasma.private.kicker as Kicker

import "code/tools.js" as Tools

/* TODO
 * Reverse middleRow layout + keyboard nav + filter list text alignment in rtl locales.
 * Keep cursor column when arrow'ing down past non-full trailing rows into a lower grid.
 * Make DND transitions cleaner by performing an item swap instead of index reinsertion.
*/

Kicker.DashboardWindow {
    id: root

    property bool smallScreen: ((Math.floor(width / Kirigami.Units.iconSizes.huge) <= 22) || (Math.floor(height / Kirigami.Units.iconSizes.huge) <= 14))

    property int iconSize: smallScreen ? Kirigami.Units.iconSizes.large : Kirigami.Units.iconSizes.huge
    property int cellSize: iconSize + (2 * Kirigami.Units.iconSizes.sizeForLabels)
        + (2 * Kirigami.Units.smallSpacing)
        + (2 * Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
                        highlightItemSvg.margins.left + highlightItemSvg.margins.right))
    property int columns: Math.floor(((smallScreen ? 85 : 80)/100) * Math.ceil(width / cellSize))
    property bool searching: searchField.text !== ""

    keyEventProxy: searchField

    backgroundColor: "transparent"

    onKeyEscapePressed: {
        if (searching) {
            searchField.clear();
        } else {
            root.toggle();
        }
    }

    onActiveChanged: {
        if (!active && visible) {
            root.toggle();
        }
    }

    onVisibleChanged: {
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
        }
    }

    function reset() {
        searchField.clear();
        globalFavoritesGrid.currentIndex = -1;
        systemFavoritesGrid.currentIndex = -1;
        filterList.currentIndex = 0;
        funnelModel.sourceModel = rootModel.modelForRow(0);
        mainGrid.model = funnelModel;
        mainGrid.currentIndex = -1;
        filterList.model = rootModel;
    }

    mainItem: MouseArea {
        id: rootItem

        anchors.fill: parent

        Kirigami.Theme.colorSet: Plasmoid.configuration.forceDarkMode ? Kirigami.Theme.Complementary : Kirigami.Theme.Window
        Kirigami.Theme.inherit: false

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        // We draw the background color here instead of in the
        // DashboardWindow as transparent background window
        // color do not play well with non-black backgrounds,
        // as the color spills in the blur effect even with
        // complete transparency.
        Rectangle {
            anchors.fill: parent
            color: Kirigami.Theme.backgroundColor
            opacity: 0.737
        }

        Connections {
            target: kicker

            function onReset() {
                if (!root.searching) {
                    filterList.applyFilter();
                    funnelModel.reset();
                }
            }

            function onDragSourceChanged() {
                if (!kicker.dragSource) {
                    // FIXME TODO HACK: Reset all views post-DND to work around
                    // mouse grab bug despite QQuickWindow::mouseGrabberItem==0x0.
                    // Needs a more involved hunt through Qt Quick sources later since
                    // it's not happening with near-identical code in the menu repr.
                    rootModel.refresh();
                }
            }
        }

        Connections {
            target: Plasmoid
            function onUserConfiguringChanged() {
                if (Plasmoid.userConfiguring) {
                    root.hide()
                }
            }
        }

        PlasmaExtras.Menu {
            id: contextMenu

            PlasmaExtras.MenuItem {
                action: Plasmoid.internalAction("configure")
            }
        }

        Kirigami.Heading {
            id: dummyHeading

            visible: false

            width: 0

            level: 1
            textFormat: Text.PlainText
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

        PlasmaExtras.SearchField {
            id: searchField

            anchors {
                horizontalCenter: parent.horizontalCenter
            }

            y: (middleRow.anchors.topMargin / 2) - (root.smallScreen ? (height/10) : 0)
            width: Kirigami.Units.gridUnit * 24
            font.pointSize: dummyHeading.font.pointSize * 1.5

            onTextChanged: {
                runnerModel.query = searchField.text
                this.forceActiveFocus()
            }

            onFocusChanged: {
                if (!focus)
                    runnerGrid.firstGrid.currentIndex = -1
            }

            Keys.forwardTo: runnerGrid.visible && runnerGrid.firstGrid ? [runnerGrid.firstGrid.view] : []

            function clear() {
                text = "";
            }

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

                    if (globalFavoritesGrid.enabled) {
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
                topMargin: Kirigami.Units.gridUnit * (smallScreen ? 8 : 10)
                bottom: parent.bottom
                bottomMargin: (Kirigami.Units.gridUnit * 2)
                horizontalCenter: parent.horizontalCenter
            }

            width: (root.columns * root.cellSize) + (2 * spacing)

            spacing: Kirigami.Units.gridUnit * 2

            Item {
                id: favoritesColumn

                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }

                width: (columns * root.cellSize) + Kirigami.Units.gridUnit

                property int columns: Plasmoid.configuration.favoritesColumns

                Kirigami.Heading {
                    id: favoritesColumnLabel

                    anchors {
                        top: parent.top
                    }

                    x: Kirigami.Units.smallSpacing
                    width: parent.width - x

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap

                    color: Kirigami.Theme.textColor

                    level: 1

                    text: i18n("Favorites")
                    textFormat: Text.PlainText

                    opacity: enabled ? 1.0 : 0.3

                    Behavior on opacity { SmoothedAnimation { duration: Kirigami.Units.longDuration; velocity: 0.01 } }
                }

                KSvg.SvgItem {
                    id: favoritesColumnLabelUnderline

                    anchors {
                        top: favoritesColumnLabel.bottom
                    }

                    width: parent.width - Kirigami.Units.gridUnit

                    imagePath: "widgets/line"
                    elementId: "horizontal-line"

                    opacity: enabled ? 1.0 : 0.3

                    Behavior on opacity { SmoothedAnimation { duration: Kirigami.Units.longDuration; velocity: 0.01 } }
                }

                ItemGridView {
                    id: globalFavoritesGrid

                    anchors {
                        top: favoritesColumnLabelUnderline.bottom
                        topMargin: Kirigami.Units.gridUnit
                    }

                    property int rows: (Math.floor((parent.height - favoritesColumnLabel.height
                        - favoritesColumnLabelUnderline.height - Kirigami.Units.gridUnit) / root.cellSize)
                        - systemFavoritesGrid.rows)

                    width: parent.width
                    height: rows * root.cellSize

                    cellWidth: root.cellSize
                    cellHeight: root.cellSize
                    iconSize: root.iconSize

                    model: globalFavorites

                    dropEnabled: true

                    opacity: enabled ? 1.0 : 0.3

                    Behavior on opacity { SmoothedAnimation { duration: Kirigami.Units.longDuration; velocity: 0.01 } }

                    onCurrentIndexChanged: {
                        preloadAllAppsTimer.defer();
                    }

                    onKeyNavRight: subGridIndex => {
                        mainColumn.tryActivate(currentRow(), 0);
                    }

                    onKeyNavDown: {
                        systemFavoritesGrid.tryActivate(0, currentCol());
                    }

                    Keys.onPressed: event => {
                        if (event.key === Qt.Key_Tab) {
                            event.accepted = true;

                            if (root.searching) {
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
                id: splitter
                width: Math.round(Kirigami.Units.gridUnit / 2)
                anchors.top: parent.top
                anchors.bottom: parent.bottom

                MouseArea {
                    anchors.fill: parent
                    anchors.leftMargin: -middleRow.spacing
                    anchors.rightMargin: -middleRow.spacing
                    cursorShape: Qt.SizeHorCursor
                    drag.target: parent
                    drag.axis: Drag.XAxis

                    property int startX
                    property int startFavCols

                    onPressed: {
                        startX = splitter.x
                        startFavCols = favoritesColumn.columns
                    }
                    onPositionChanged: {
                        let deltaCols = Math.round((splitter.x - startX) / root.cellSize)
                        let newFavCols = startFavCols + deltaCols
                        if (newFavCols >= 1 && newFavCols < root.columns - filterListColumn.columns) {
                            favoritesColumn.columns = newFavCols
                            Plasmoid.configuration.favoritesColumns = newFavCols
                        }
                    }
                }
            }

            Item {
                id: mainColumn

                anchors.top: parent.top

                width: (columns * root.cellSize) + Kirigami.Units.gridUnit
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

                    property int headerHeight: mainColumnLabel.height + mainColumnLabelUnderline.height + Kirigami.Units.gridUnit

                    opacity: {
                        if (root.searching) {
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

                    Kirigami.Heading {
                        id: mainColumnLabel

                        anchors {
                            top: parent.top
                        }

                        x: Kirigami.Units.smallSpacing
                        width: parent.width - x

                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                        opacity: 1.0

                        color: Kirigami.Theme.textColor

                        level: 1

                        text: funnelModel.description
                        textFormat: Text.PlainText
                    }

                    KSvg.SvgItem {
                        id: mainColumnLabelUnderline

                        visible: mainGrid.count

                        anchors {
                            top: mainColumnLabel.bottom
                        }

                        width: parent.width - Kirigami.Units.gridUnit

                        imagePath: "widgets/line"
                        elementId: "horizontal-line"
                    }

                    ItemGridView {
                        id: mainGrid

                        anchors {
                            top: mainColumnLabelUnderline.bottom
                            topMargin: Kirigami.Units.gridUnit
                        }

                        width: parent.width
                        height: systemFavoritesGrid.y + systemFavoritesGrid.height - mainGridContainer.headerHeight

                        cellWidth: root.cellSize
                        cellHeight: cellWidth
                        iconSize: root.iconSize

                        model: funnelModel

                        onCurrentIndexChanged: {
                            preloadAllAppsTimer.defer();
                        }

                        onKeyNavLeft: {
                            var row = currentRow();
                            var target = row + 1 > globalFavoritesGrid.rows ? systemFavoritesGrid : globalFavoritesGrid;
                            var targetRow = row + 1 > globalFavoritesGrid.rows ? row - globalFavoritesGrid.rows : row;
                            target.tryActivate(targetRow, favoritesColumn.columns - 1);
                        }

                        onKeyNavRight: {
                            filterListScrollArea.focus = true;
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

                    onKeyNavLeft: subGridIndex => {
                        var row = 0;

                        for (var i = 0; i < subGridIndex; i++) {
                            row += subGridAt(i).lastRow() + 2; // Header counts as one.
                        }

                        row += subGridAt(subGridIndex).currentRow();

                        var target = row + 1 > globalFavoritesGrid.rows ? systemFavoritesGrid : globalFavoritesGrid;
                        var targetRow = row + 1 > globalFavoritesGrid.rows ? row - globalFavoritesGrid.rows : row;
                        target.tryActivate(targetRow, favoritesColumn.columns - 1);
                    }

                    onKeyNavRight: subGridIndex => {
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

                    grabFocus: false

                    opacity: root.searching ? 1.0 : 0.0

                    onOpacityChanged: {
                        if (opacity === 1.0) {
                            mainColumn.visibleGrid = runnerGrid;
                        }
                    }

                    onKeyNavLeft: subGridIndex => {
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
                        } else if (globalFavoritesGrid.enabled) {
                            globalFavoritesGrid.tryActivate(0, 0);
                        } else {
                            systemFavoritesGrid.tryActivate(0, 0);
                        }
                    } else {
                        event.accepted = false;
                    }
                }
            }

            Item {
                id: filterListColumn

                anchors {
                    top: parent.top
                    topMargin: mainColumnLabelUnderline.y + mainColumnLabelUnderline.height + Kirigami.Units.gridUnit
                    bottom: parent.bottom
                }

                width: columns * root.cellSize

                property int columns: 3

                PlasmaComponents.ScrollView {
                    id: filterListScrollArea

                    x: root.visible ? 0 : Kirigami.Units.gridUnit

                    Behavior on x { SmoothedAnimation { duration: Kirigami.Units.longDuration; velocity: 0.01 } }

                    width: parent.width
                    height: mainGrid.height

                    enabled: !root.searching

                    property alias currentIndex: filterList.currentIndex

                    opacity: root.visible ? (root.searching ? 0.30 : 1.0) : 0.3

                    Behavior on opacity { SmoothedAnimation { duration: Kirigami.Units.longDuration; velocity: 0.01 } }

                    PlasmaComponents.ScrollBar.vertical.policy: (opacity === 1.0) ? PlasmaComponents.ScrollBar.AsNeeded : PlasmaComponents.ScrollBar.AlwaysOff

                    onEnabledChanged: {
                        if (!enabled) {
                            filterList.currentIndex = -1;
                        }
                    }

                    ListView {
                        id: filterList

                        focus: true

                        property bool allApps: false
                        property int eligibleWidth: width
                        property int hItemMargins: Math.max(highlightItemSvg.margins.left + highlightItemSvg.margins.right,
                            listItemSvg.margins.left + listItemSvg.margins.right)

                        model: rootModel

                        clip: height < contentHeight + topMargin + bottomMargin
                        boundsBehavior: Flickable.StopAtBounds
                        snapMode: ListView.SnapToItem
                        spacing: 0
                        keyNavigationWraps: true

                        delegate: MouseArea {
                            id: item

                            signal actionTriggered(string actionId, var actionArgument)
                            signal aboutToShowActionMenu(var actionMenu)

                            property var m: model
                            property int textWidth: label.contentWidth
                            property int mouseCol
                            property bool hasActionList: ((model.favoriteId !== null)
                                || (("hasActionList" in model) && (model.hasActionList === true)))
                            property ActionMenu menu: actionMenu

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

                                if (ListView.view.currentIndex === 0 || index === ListView.view.currentIndex) {
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

                            onAboutToShowActionMenu: actionMenu => {
                                var actionList = hasActionList ? model.actionList : [];
                                Tools.fillActionMenu(i18n, actionMenu, actionList, ListView.view.model.favoritesModel, model.favoriteId);
                            }

                            onActionTriggered: (actionId, actionArgument) => {
                                if (Tools.triggerAction(ListView.view.model, model.index, actionId, actionArgument) === true) {
                                    kicker.expanded = false;
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

                                onActionClicked: (actionId, actionArgument) => {
                                    actionTriggered(actionId, actionArgument);
                                }
                            }

                            Timer {
                                id: updateCurrentItemTimer

                                interval: 50
                                repeat: false

                                onTriggered: parent.updateCurrentItem()
                            }

                            Kirigami.Heading {
                                id: label

                                anchors {
                                    fill: parent
                                    topMargin: highlightItemSvg.margins.top
                                    bottomMargin: highlightItemSvg.margins.bottom
                                    leftMargin: highlightItemSvg.margins.left
                                    rightMargin: highlightItemSvg.margins.right
                                }

                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                                opacity: 1.0

                                color: Kirigami.Theme.textColor

                                level: 1

                                text: model.display
                                textFormat: Text.PlainText
                            }
                        }

                        highlight: PlasmaExtras.Highlight {
                            x: LayoutMirroring.enabled ? filterList.width - width : 0
                            y: filterList.currentItem ? filterList.currentItem.y : 0
                            height: filterList.currentItem ? filterList.currentItem.height : 0
                            width: (highlightItemSvg.margins.left
                                + (filterList.currentItem ? filterList.currentItem.textWidth : 0)
                                + highlightItemSvg.margins.right
                                + Kirigami.Units.smallSpacing)

                            visible: filterList.currentItem
                            active: filterListScrollArea.focus
                            pressed: filterList.currentItem && filterList.currentItem.pressed
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
                            filterListScrollArea.width = width + hItemMargins + (Kirigami.Units.gridUnit * 2);
                        }

                        function applyFilter() {
                            if (!root.searching && currentIndex >= 0) {
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
                            let backArrowKey = (event.key === Qt.Key_Left && Application.layoutDirection === Qt.LeftToRight) ||
                                (event.key === Qt.Key_Right && Application.layoutDirection === Qt.RightToLeft)
                            if (backArrowKey) {
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
