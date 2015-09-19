/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                        *
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

import QtQuick 2.4
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

import org.kde.plasma.private.kicker 0.1 as Kicker

import "../code/tools.js" as Tools

/* TODO
 * Reverse middleRow layout + keyboard nav + filter list text alignment in rtl locales.
 * Keep cursor column when arrow'ing down past non-full trailing rows into a lower grid.
 * Make DND transitions cleaner by performing an item swap instead of index reinsertion.
*/

Kicker.FullScreenWindow {
    id: root

    property bool smallScreen: (Math.floor(width / units.iconSizes.huge) <= 22)
    property int iconSize: smallScreen ? units.iconSizes.large : units.iconSizes.huge
    property int cellSize: iconSize + theme.mSize(theme.defaultFont).height
        + (2 * units.smallSpacing)
        + (2 * Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
                        highlightItemSvg.margins.left + highlightItemSvg.margins.right))
    property int columns: Math.floor(((smallScreen ? 85 : 80)/100) * Math.ceil(width / cellSize))
    property bool searching: (searchField.text != "")

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
        mainGrid.currentIndex = -1;
        filterListScrollArea.focus = true;
    }

    mainItem: MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.LeftButton | Qt.RightButton

        Connections {
            target: kicker

            onReset: {
                if (!searching) {
                    filterList.applyFilter();
                    funnelModel.reset();
                }
            }

            onDragSourceChanged: {
                if (!dragSource) {
                    // FIXME TODO HACK: Reset favorites grids post-DND to work around
                    // mouse grab bug despite QQuickWindow::mouseGrabberItem==0x0.
                    // Needs a more involved hunt through Qt Quick sources later since
                    // it's not happening with near-identical code in the menu repr.
                    var index = globalFavoritesGrid.currentIndex;
                    globalFavoritesGrid.model = null;
                    globalFavoritesGrid.model = globalFavorites;
                    globalFavoritesGrid.currentIndex = index;
                    index = systemFavoritesGrid.currentIndex;
                    systemFavoritesGrid.model = null;
                    systemFavoritesGrid.model = systemFavorites;
                    systemFavoritesGrid.currentIndex = index;
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
                if (done || searching) {
                    return;
                }

                for (var i = 0; i < rootModel.count; ++i) {
                    var model = rootModel.modelForRow(i);

                    if (model.description == "KICKER_ALL_MODEL") {
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

        PlasmaComponents.TextField {
            id: searchField

            width: 0
            height: 0

            visible: false

            property string bufferedText

            onTextChanged: {
                runnerModel.query = text;
            }

            function appendText(appendText) {
                if (text == "" || searchTimer.running) {
                    bufferedText += appendText;
                    searchTimer.start();
                } else {
                    text += appendText;
                }
            }

            function backspace() {
                if (searchTimer.running) {
                    bufferedText = bufferedText.slice(0, -1);
                } else {
                    text = text.slice(0, -1);
                }
            }

            function clear() {
                text = "";
            }

            Timer {
                id: searchTimer

                interval: 150
                repeat: false

                onTriggered: {
                    searchField.focus = true;
                    searchField.text = searchField.bufferedText;
                    searchField.bufferedText = "";
                }
            }
        }

        PlasmaExtras.Heading {
            id: searchHeading

            anchors {
                horizontalCenter: parent.horizontalCenter
            }

            y: (middleRow.anchors.topMargin / 2) - (smallScreen ? (height/2) : 0)

            font.pointSize: dummyHeading.font.pointSize * 1.5

            elide: Text.ElideRight
            wrapMode: Text.NoWrap
            opacity: 1.0

            color: "white"

            level: 1

            text: searching ? i18n('Searching for "%1"', searchField.text) : i18n("Type to search.")
        }

        PlasmaComponents.ToolButton {
            id: cancelSearchButton

            anchors {
                left: searchHeading.right
                leftMargin: units.largeSpacing
                verticalCenter: searchHeading.verticalCenter
            }

            width: units.iconSizes.large
            height: width

            visible: (searchField.text != "")

            iconName: "dialog-close"
            flat: false

            onClicked: searchField.clear();
        }

        Row {
            id: middleRow

            anchors {
                top: parent.top
                topMargin: units.gridUnit * (smallScreen ? 6 : 10)
                bottom: parent.bottom
                bottomMargin: (units.gridUnit * 2)
                horizontalCenter: parent.horizontalCenter
            }

            width: (root.columns * cellSize) + (2 * spacing)

            spacing: units.gridUnit * 2

            Item {
                id: favoritesColumn

                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }

                width: (columns * cellSize) + units.gridUnit

                property int columns: 3

                PlasmaExtras.Heading {
                    id: favoritesColumnLabel

                    anchors {
                        top: parent.top
                    }

                    x: units.smallSpacing
                    width: parent.width - x

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    opacity: 1.0

                    color: "white"

                    level: 1

                    text: i18n("Favorites")
                }

                PlasmaCore.SvgItem {
                    id: favoritesColumnLabelUnderline

                    anchors {
                        top: favoritesColumnLabel.bottom
                    }

                    width: parent.width - units.gridUnit
                    height: lineSvg.horLineHeight

                    svg: lineSvg
                    elementId: "horizontal-line"
                }

                ItemGridView {
                    id: globalFavoritesGrid

                    anchors {
                        top: favoritesColumnLabelUnderline.top
                        topMargin: units.largeSpacing
                    }

                    property int rows: (Math.floor((parent.height - favoritesColumnLabel.height
                        - favoritesColumnLabelUnderline.height - units.largeSpacing) / cellSize)
                        - systemFavoritesGrid.rows)

                    width: parent.width
                    height: rows * cellSize

                    cellWidth: cellSize
                    cellHeight: cellSize

                    model: globalFavorites

                    dragEnabled: true

                    onCurrentIndexChanged: {
                        preloadAllAppsTimer.defer();
                    }

                    onKeyNavRight: {
                        mainColumn.tryActivate(currentRow(), 0);
                    }

                    onKeyNavDown: {
                        systemFavoritesGrid.tryActivate(0, currentCol());
                    }

                    Binding {
                        target: globalFavorites
                        property: "iconSize"
                        value: root.iconSize
                    }
                }

                ItemGridView {
                    id: systemFavoritesGrid

                    anchors {
                        top: globalFavoritesGrid.bottom
                    }

                    property int rows: Math.ceil(count / Math.floor(width / cellSize))

                    width: parent.width
                    height: rows * cellSize

                    cellWidth: cellSize
                    cellHeight: cellSize

                    model: systemFavorites

                    dragEnabled: true

                    onCurrentIndexChanged: {
                        preloadAllAppsTimer.defer();
                    }

                    onKeyNavRight: {
                        mainColumn.tryActivate(globalFavoritesGrid.rows + currentRow(), 0);
                    }

                    onKeyNavUp: {
                        globalFavoritesGrid.tryActivate(globalFavoritesGrid.rows - 1, currentCol());
                    }
                }
            }

            Item {
                id: mainColumn

                anchors.top: parent.top

                width: (columns * cellSize) + units.gridUnit
                height: Math.floor(parent.height / cellSize) * cellSize + mainGridContainer.headerHeight

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
                    z: (opacity == 1.0) ? 1 : 0

                    enabled: (opacity == 1.0) ? 1 : 0

                    property int headerHeight: mainColumnLabel.height + mainColumnLabelUnderline.height + units.largeSpacing

                    opacity: (!searching && !filterList.allApps) ? 1.0 : 0.0

                    onOpacityChanged: {
                        if (opacity == 1.0) {
                            mainColumn.visibleGrid = mainGrid;
                        }
                    }

                    PlasmaExtras.Heading {
                        id: mainColumnLabel

                        anchors {
                            top: parent.top
                        }

                        x: units.smallSpacing
                        width: parent.width - x

                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                        opacity: 1.0

                        color: "white"

                        level: 1

                        text: funnelModel.description
                    }

                    PlasmaCore.SvgItem {
                        id: mainColumnLabelUnderline

                        visible: mainGrid.count

                        anchors {
                            top: mainColumnLabel.bottom
                        }

                        width: parent.width - units.gridUnit
                        height: lineSvg.horLineHeight

                        svg: lineSvg
                        elementId: "horizontal-line"
                    }

                    ItemGridView {
                        id: mainGrid

                        anchors {
                            top: mainColumnLabelUnderline.bottom
                            topMargin: units.largeSpacing
                        }

                        width: parent.width
                        height: systemFavoritesGrid.y + systemFavoritesGrid.height - mainGridContainer.headerHeight

                        cellWidth: cellSize
                        cellHeight: cellSize

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

                    z: (opacity == 1.0) ? 1 : 0
                    width: parent.width
                    height: systemFavoritesGrid.y + systemFavoritesGrid.height

                    enabled: (opacity == 1.0) ? 1 : 0

                    model: runnerModel

                    opacity: filterList.allApps ? 1.0 : 0.0

                    onOpacityChanged: {
                        if (opacity == 1.0) {
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

                    z: (opacity == 1.0) ? 1 : 0
                    width: parent.width
                    height: systemFavoritesGrid.y + systemFavoritesGrid.height

                    enabled: (opacity == 1.0) ? 1 : 0

                    model: runnerModel

                    grabFocus: true

                    opacity: searching ? 1.0 : 0.0

                    onOpacityChanged: {
                        if (opacity == 1.0) {
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
            }

            Item {
                id: filterListColumn

                anchors {
                    top: parent.top
                    topMargin: mainColumnLabelUnderline.y + mainColumnLabelUnderline.height + units.largeSpacing
                    bottom: parent.bottom
                }

                width: columns * cellSize

                property int columns: 3

                PlasmaExtras.ScrollArea {
                    id: filterListScrollArea

                    x: root.visible ? 0 : units.gridUnit

                    Behavior on x { SmoothedAnimation { duration: units.longDuration; velocity: 0.01 } }

                    width: parent.width
                    height: mainGrid.height

                    enabled: !searching

                    property alias currentIndex: filterList.currentIndex

                    opacity: root.visible ? (searching ? 0.30 : 1.0) : 0.3

                    Behavior on opacity { SmoothedAnimation { duration: units.longDuration; velocity: 0.01 } }

                    verticalScrollBarPolicy: (opacity == 1.0) ? Qt.ScrollBarAsNeeded : Qt.ScrollBarAlwaysOff

                    onEnabledChanged: {
                        if (!enabled) {
                            filterList.currentIndex = -1;
                        }
                    }

                    onCurrentIndexChanged: {
                        focus = (currentIndex != -1);
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

                        delegate: MouseArea {
                            id: item

                            signal actionTriggered(string actionId, variant actionArgument)
                            signal aboutToShowActionMenu(variant actionMenu)

                            property int textWidth: label.contentWidth
                            property int mouseCol
                            property bool hasActionList: ((model.favoriteId != null)
                                || (("hasActionList" in model) && (model.hasActionList == true)))
                            property Item menu: actionMenu

                            width: parent.width
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

                            onPositionChanged: { // Lazy menu implementation.
                                mouseCol = mouse.x;

                                if (justOpenedTimer.running || ListView.view.currentIndex == 0 || index == ListView.view.currentIndex) {
                                    updateCurrentItem();
                                } else if ((index == ListView.view.currentIndex - 1) && mouse.y < (height - 6)
                                    || (index == ListView.view.currentIndex + 1) && mouse.y > 5) {

                                    if (mouse.x > ListView.view.eligibleWidth - 5) {
                                        updateCurrentItem();
                                    }
                                } else if (mouse.x > ListView.view.eligibleWidth) {
                                    updateCurrentItem();
                                }

                                updateCurrentItemTimer.restart();
                            }

                            onPressed: {
                                if (mouse.buttons & Qt.RightButton) {
                                    if (hasActionList) {
                                        openActionMenu(item, mouse.x, mouse.y);
                                    }
                                }
                            }

                            onAboutToShowActionMenu: {
                                var actionList = hasActionList ? model.actionList : [];
                                Tools.fillActionMenu(actionMenu, actionList, ListView.view.model.favoritesModel, model.favoriteId);
                            }

                            onActionTriggered: {
                                Tools.triggerAction(ListView.view.model, model.index, actionId, actionArgument);
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

                        highlight: PlasmaComponents.Highlight {
                            anchors {
                                top: filterList.currentItem ? filterList.currentItem.top : undefined
                                left: filterList.currentItem ? filterList.currentItem.left : undefined
                                bottom: filterList.currentItem ? filterList.currentItem.bottom : undefined
                            }

                            opacity: filterListScrollArea.focus ? 1.0 : 0.7

                            width: (highlightItemSvg.margins.left
                                + filterList.currentItem.textWidth
                                + highlightItemSvg.margins.right
                                + units.smallSpacing)

                            visible: filterList.currentItem
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

                            filterListColumn.columns = Math.ceil(width / cellSize);
                            filterListScrollArea.width = width + hItemMargins + (units.gridUnit * 2);
                        }

                        function applyFilter() {
                            if (!searching && currentIndex >= 0) {
                                if (preloadAllAppsTimer.running) {
                                    preloadAllAppsTimer.stop();
                                }

                                var model = rootModel.modelForRow(currentIndex);

                                if (model.description == "KICKER_ALL_MODEL") {
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

                        Keys.onPressed: {
                            if (event.key == Qt.Key_Left) {
                                event.accepted = true;

                                var currentRow = Math.max(0, Math.ceil(currentItem.y / cellSize) - 1);
                                mainColumn.tryActivate(currentRow, mainColumn.columns - 1);
                            }
                        }
                    }
                }
            }
        }

        onPressed: {
            if (mouse.button == Qt.RightButton) {
                contextMenu.open(mouse.x, mouse.y);
            }
        }

        onClicked: {
            if (mouse.button == Qt.LeftButton) {
                root.toggle();
            }
        }

        Keys.onPressed: {
            if (event.key == Qt.Key_Escape) {
                event.accepted = true;

                if (searching) {
                    searchField.clear();
                } else {
                    root.toggle();
                }
            } else if (event.key == Qt.Key_Backspace) {
                event.accepted = true;
                searchField.backspace();
            } else if (event.text != "") {
                event.accepted = true;
                searchField.appendText(event.text);
            }
        }
    }
}
