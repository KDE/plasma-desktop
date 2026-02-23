/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import org.kde.plasma.private.kicker as Kicker

/* TODO
 * Reverse middleRow layout + keyboard nav + filter list text alignment in rtl locales.
 * Keep cursor column when arrow'ing down past non-full trailing rows into a lower grid.
 * Make DND transitions cleaner by performing an item swap instead of index reinsertion.
*/

Kicker.DashboardWindow {
    id: root

    required property Kicker.KAStatsFavoritesModel globalFavorites
    required property Kicker.SimpleFavoritesModel systemFavorites
    required property Kicker.RootModel rootModel
    required property Kicker.RunnerModel runnerModel

    property bool smallScreen: ((Math.floor(width / Kirigami.Units.iconSizes.huge) <= 22) || (Math.floor(height / Kirigami.Units.iconSizes.huge) <= 14))

    property int iconSize: smallScreen ? Kirigami.Units.iconSizes.large : Kirigami.Units.iconSizes.huge
    property int cellSize: iconSize + (2 * Kirigami.Units.iconSizes.sizeForLabels)
        + (2 * Kirigami.Units.smallSpacing)
        + (2 * Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
                        highlightItemSvg.margins.left + highlightItemSvg.margins.right))
    property int columns: Math.floor(((smallScreen ? 85 : 80)/100) * Math.ceil(width / cellSize))
    property bool searching: searchField.text !== ""

    signal interactionConcluded()

    keyEventProxy: searchField

    backgroundColor: "transparent"

    onKeyEscapePressed: {
        if (searching) {
            searchField.clear();
        } else {
            root.interactionConcluded()
        }
    }

    onActiveChanged: {
        if (!active && visible) {
            root.interactionConcluded()
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
        searchField.forceActiveFocus(Qt.OtherFocusReason)
        globalFavoritesGrid.currentIndex = -1;
        systemFavoritesGrid.currentIndex = -1;
        filterList.currentIndex = 0;
        filterList.applyFilter();
        mainGrid.currentIndex = -1;
        filterList.model = rootModel;
        hoverBlock.reset();
    }

    mainItem: MouseArea {
        id: rootItem

        anchors.fill: parent

        Kirigami.Theme.colorSet: Plasmoid.configuration.forceDarkMode ? Kirigami.Theme.Complementary : Kirigami.Theme.Window
        Kirigami.Theme.inherit: false

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        LayoutMirroring.enabled: Application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        // We draw the background color here instead of in the
        // DashboardWindow as transparent background window
        // color do not play well with non-black backgrounds,
        // as the color spills in the blur effect even with
        // complete transparency.
        Rectangle {
            anchors.fill: parent
            // Intentionally hardcoded to black in dark mode because any other color
            // looks terrible here. Any bug reports about illegible text or icons
            // should be considered a color scheme error and sent back to the user
            // or their distro.
            color: Plasmoid.configuration.forceDarkMode ? "black" : Kirigami.Theme.backgroundColor
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

                for (var i = 0; i < root.rootModel.count; ++i) {
                    var model = root.rootModel.modelForRow(i);

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
            focus: true

            onTextChanged: {
                root.runnerModel.query = searchField.text
                this.forceActiveFocus()
            }

            onFocusChanged: {
                if (!focus && runnerGrid?.firstGrid) {
                    runnerGrid.firstGrid.currentIndex = -1
                }
            }

            function clear() {
                text = "";
            }

            Keys.priority: Keys.AfterItem
            Keys.onTabPressed: {
                if (root.runnerModel.count) {
                    focus = false
                    mainColumn.tryActivate(0, 0);
                } else if (mainGrid.visible && mainGrid.count) {
                    mainGrid.tryActivate(0, 0);
                } else if (allAppsGrid.visible && allAppsGrid.count) {
                    allAppsGrid.tryActivate(0, 0);
                } else {
                    systemFavoritesGrid.tryActivate(0, 0);
                }
            }
            Keys.onBacktabPressed: {
                if (globalFavoritesGrid.enabled) {
                    globalFavoritesGrid.tryActivate(0, 0);
                } else {
                    systemFavoritesGrid.tryActivate(0, 0);
                }
            }
            Keys.onDownPressed: event => {
                if (mainGrid.visible) {
                    mainGrid.tryActivate(0, 0);
                } else if (allAppsGrid.visible) {
                    allAppsGrid.tryActivate(0, 0);
                } else if (runnerGrid.visible) {
                    if (runnerGrid.firstGrid.currentIndex != -1) {
                        runnerGrid.firstGrid.view.moveCurrentIndexDown()
                        let currentRow = runnerGrid.firstGrid.currentRow()
                        let currentCol = runnerGrid.firstGrid.currentCol()
                        focus = false
                        runnerGrid.tryActivate(currentRow, currentCol)
                    } else {
                        focus = false
                        runnerGrid.tryActivate(0,0)
                    }
                } else {
                    event.accepted = false
                }
            }
            Keys.onRightPressed: event => {
                if (runnerGrid.visible && runnerGrid.firstGrid.currentIndex != -1 &&
                    (Application.layoutDirection == Qt.LeftToRight || runnerGrid.firstGrid.currentIndex != 0)) {
                    runnerGrid.firstGrid.view.moveCurrentIndexRight()
                    let currentRow = runnerGrid.firstGrid.currentRow()
                    let currentCol = runnerGrid.firstGrid.currentCol()
                    focus = false
                    runnerGrid.tryActivate(currentRow, currentCol)
                } else {
                    event.accepted = false
                }
            }
            Keys.onLeftPressed: event => {
                if (runnerGrid.visible && runnerGrid.firstGrid.currentIndex != -1 &&
                    (Application.layoutDirection == Qt.RightToLeft || runnerGrid.firstGrid.currentIndex != 0)) {
                    runnerGrid.firstGrid.view.moveCurrentIndexLeft()
                    let currentRow = runnerGrid.firstGrid.currentRow()
                    let currentCol = runnerGrid.firstGrid.currentCol()
                    focus = false
                    runnerGrid.tryActivate(currentRow, currentCol)
                    } else {
                        event.accepted = false
                    }
            }
            Keys.onReturnPressed: event => {
                // this mostly should apply to the runner grid, but there are a few ways to
                // have focus on the search field and have something visually selected, so
                // fall back to the other possible grids
                let currentDelegate = runnerGrid.visible  ? runnerGrid.firstGrid?.currentItem :
                                      allAppsGrid.visible ? allAppsGrid.firstGrid?.currentItem :
                                                            mainGrid.currentItem
                currentDelegate.Keys.returnPressed(event)
            }
            Keys.onEnterPressed: event => Keys.returnPressed(event)
        }

        Row {
            id: middleRow

            anchors {
                top: parent.top
                topMargin: Kirigami.Units.gridUnit * (root.smallScreen ? 8 : 10)
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

                    text: i18nc("@title:group for column with app and system action grids", "Favorites")
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

                    model: root.globalFavorites

                    hoverEnabled: !hoverBlock.enabled
                    dropEnabled: true

                    opacity: enabled ? 1.0 : 0.3

                    Behavior on opacity { SmoothedAnimation { duration: Kirigami.Units.longDuration; velocity: 0.01 } }

                    onInteractionConcluded: root.interactionConcluded()
                    onCurrentIndexChanged: {
                        preloadAllAppsTimer.defer();
                    }

                    onKeyNavRight: {
                        mainColumn.tryActivate(currentRow(), 0);
                    }

                    onKeyNavDown: {
                        systemFavoritesGrid.tryActivate(0, currentCol());
                    }

                    Keys.onTabPressed: {
                        currentIndex = -1
                        searchField.forceActiveFocus(Qt.TabFocusReason)
                    }
                    Keys.onBacktabPressed: systemFavoritesGrid.tryActivate(0, 0);

                    Binding {
                        target: root.globalFavorites
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

                    model: root.systemFavorites

                    hoverEnabled: !hoverBlock.enabled
                    dropEnabled: true

                    onInteractionConcluded: root.interactionConcluded()
                    onCurrentIndexChanged: {
                        preloadAllAppsTimer.defer();
                    }

                    onKeyNavRight: {
                        mainColumn.tryActivate(globalFavoritesGrid.rows + currentRow(), 0);
                    }

                    onKeyNavUp: {
                        globalFavoritesGrid.tryActivate(globalFavoritesGrid.rows - 1, currentCol());
                    }

                    Keys.onTabPressed: {
                        if (globalFavoritesGrid.enabled) {
                            globalFavoritesGrid.tryActivate(0, 0);
                        } else {
                            searchField.forceActiveFocus(Qt.TabFocusReason)
                        }
                    }
                    Keys.onBacktabPressed: {
                        if (filterList.enabled) {
                            filterList.forceActiveFocus();
                        } else {
                            mainColumn.tryActivate(0, 0);
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

                        hoverEnabled: !hoverBlock.enabled

                        model: funnelModel

                        onInteractionConcluded: root.interactionConcluded()
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

                        onKeyNavUp: {
                            currentIndex = -1
                            searchField.forceActiveFocus(Qt.TabFocusReason)
                        }

                        Keys.onBacktabPressed: event => {
                            currentIndex = -1
                            event.accepted = false // pass to mainColumn handler
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
                    cellSize: root.cellSize
                    iconSize: root.iconSize

                    hoverEnabled: !hoverBlock.enabled

                    visible: opacity !== 0.0

                    opacity: filterList.allApps && !root.searching ? 1.0 : 0.0

                    onInteractionConcluded: root.interactionConcluded()
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

                    onKeyNavUp: {
                        firstGrid.currentIndex = -1
                        searchField.forceActiveFocus(Qt.TabFocusReason)
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
                    cellSize: root.cellSize
                    iconSize: root.iconSize

                    visible: opacity !== 0.0

                    model: root.runnerModel

                    hoverEnabled: !hoverBlock.enabled
                    grabFocus: false

                    opacity: root.searching ? 1.0 : 0.0

                    onInteractionConcluded: root.interactionConcluded()
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

                    onKeyNavUp: {
                        firstGrid.currentIndex = -1
                        searchField.forceActiveFocus(Qt.TabFocusReason)
                    }
                }

                Keys.onTabPressed: {
                    if (filterList.enabled) {
                        filterList.forceActiveFocus();
                    } else {
                        systemFavoritesGrid.tryActivate(0, 0);
                    }
                }
                Keys.onBacktabPressed: {
                    searchField.forceActiveFocus(Qt.BacktabFocusReason)
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
                    hoverEnabled: !hoverBlock.enabled

                    property alias currentIndex: filterList.currentIndex

                    opacity: root.visible ? (root.searching ? 0.30 : 1.0) : 0.3

                    Behavior on opacity { SmoothedAnimation { duration: Kirigami.Units.longDuration; velocity: 0.01 } }

                    PlasmaComponents.ScrollBar.vertical.policy: (opacity === 1.0) ? PlasmaComponents.ScrollBar.AsNeeded : PlasmaComponents.ScrollBar.AlwaysOff

                    onEnabledChanged: {
                        if (!enabled) {
                            filterList.currentIndex = -1;
                        }
                    }

                    onHoveredChanged: {
                        if (!hovered) {
                            filterList.currentIndex = filterList.activeIndex
                            switchFilterTimer.stop()
                        }
                    }

                    ListView {
                        id: filterList

                        focus: true

                        property bool allApps: false
                        property int activeIndex: -1
                        property int hItemMargins: Math.max(highlightItemSvg.margins.left + highlightItemSvg.margins.right,
                            listItemSvg.margins.left + listItemSvg.margins.right)

                        model: root.rootModel

                        clip: height < contentHeight + topMargin + bottomMargin
                        boundsBehavior: Flickable.StopAtBounds
                        snapMode: ListView.SnapToItem
                        spacing: 0
                        keyNavigationWraps: true

                        delegate: ItemAbstractDelegate {
                            id: item

                            property var m: model
                            property int textWidth: label.contentWidth

                            width: ListView.view.width
                            height: implicitHeight
                            hoverEnabled: !hoverBlock.enabled
                            baseModel: filterList.model
                            favoritesModel: baseModel.favoritesModel

                            onClicked: ListView.view.applyFilter()

                            onHoveredChanged: {
                                if (hovered && !isSeparator) {
                                    filterList.currentIndex = index
                                    filterList.forceActiveFocus()
                                    switchFilterTimer.restart()
                                } else if (!hovered && filterList.currentIndex === index) {
                                    switchFilterTimer.stop()
                                }
                            }

                            contentItem: Kirigami.Heading {
                                id: label

                                elide: Text.ElideRight
                                wrapMode: Text.NoWrap
                                opacity: 1.0

                                color: Kirigami.Theme.textColor

                                level: 1

                                text: item.model.display
                                textFormat: Text.PlainText
                            }
                        }

                        highlight: PlasmaExtras.Highlight {
                            visible: filterList.currentItem
                            active: filterListScrollArea.focus
                            pressed: filterList.currentItem && (filterList.currentItem as ItemAbstractDelegate).pressed
                        }

                        highlightMoveDuration: 0
                        highlightResizeDuration: 0

                        onCountChanged: {
                            var width = 0;

                            for (var i = 0; i < root.rootModel.count; ++i) {
                                headingMetrics.text = root.rootModel.labelForRow(i);

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

                                var model = root.rootModel.modelForRow(currentIndex);

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
                            filterList.activeIndex = currentIndex;
                        }

                        function handleLeftRightArrow(event: KeyEvent) : void {
                            let backArrowKey = (event.key === Qt.Key_Left && Application.layoutDirection === Qt.LeftToRight) ||
                                (event.key === Qt.Key_Right && Application.layoutDirection === Qt.RightToLeft)
                            if (backArrowKey) {
                                const currentRow = Math.max(0, Math.ceil(currentItem.y / mainGrid.cellHeight) - 1);
                                mainColumn.tryActivate(currentRow, mainColumn.columns - 1);
                            } else {
                                event.accepted = false
                            }
                        }

                        Keys.onTabPressed:  systemFavoritesGrid.tryActivate(0, 0);
                        Keys.onBacktabPressed: mainColumn.tryActivate(0, 0);
                        Keys.onLeftPressed: event => handleLeftRightArrow(event)
                        Keys.onRightPressed: event => handleLeftRightArrow(event)

                        Timer {
                            id: switchFilterTimer

                            interval: 150
                            repeat: false

                            onTriggered: filterList.applyFilter()
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
                root.interactionConcluded()
            }
        }

        MouseArea {
            id: hoverBlock  // don't hover-activate until mouse is moved to not interfere with keyboard use
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true // clicking should still work if hovering is blocked

            property bool mouseMoved: false

            function reset() {
                mouseMoved = false
                enabled = true
            }

            onPositionChanged: if (!mouseMoved) {
                mouseMoved = true
            } else {
                enabled = false // this immediately triggers other hover events when bound to their hoverEnabled
            }
        }
    }

    Component.onCompleted: {
        rootModel.refresh();
    }
}
