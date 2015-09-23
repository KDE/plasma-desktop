/***************************************************************************
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
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

import QtQuick 2.3
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

import org.kde.private.desktopcontainment.folder 0.1 as Folder
import "FolderTools.js" as FolderTools

Item {
    id: main

    signal pressed

    property QtObject model: dir
    property Item rubberBand: null

    property alias isRootView: gridView.isRootView
    property alias currentIndex: gridView.currentIndex
    property alias url: dir.url
    property alias positions: positioner.positions
    property alias errorString: dir.errorString
    property alias locked: dir.locked
    property alias sortMode: dir.sortMode
    property alias filterMode: dir.filterMode
    property alias filterPattern: dir.filterPattern
    property alias filterMimeTypes: dir.filterMimeTypes
    property alias flow: gridView.flow
    property alias layoutDirection: gridView.layoutDirection
    property alias cellWidth: gridView.cellWidth
    property alias cellHeight: gridView.cellHeight
    property alias overflowing: gridView.overflowing
    property alias scrollLeft: gridView.scrollLeft
    property alias scrollRight: gridView.scrollRight
    property alias scrollUp: gridView.scrollUp
    property alias scrollDown: gridView.scrollDown
    property Item upButton: null

    function linkHere(sourceUrl) {
        dir.linkHere(sourceUrl);
    }

    function dropItemAt(pos) {
        var item = gridView.itemAt(pos.x, pos.y);

        if (item) {
            if (item.blank) {
                return -1;
            }

            var hPos = mapToItem(item.hoverArea, pos.x, pos.y);

            if ((hPos.x < 0 || hPos.y < 0 || hPos.x > item.hoverArea.width || hPos.y > item.hoverArea.height)) {
                return -1;
            } else {
                return positioner.map(item.index);
            }
        }

        return -1;
    }

    function drop(target, event, pos) {
        var dropPos = mapToItem(gridView.contentItem, pos.x, pos.y);
        var dropIndex = gridView.indexAt(dropPos.x, dropPos.y);
        var dragPos = mapToItem(gridView.contentItem, listener.dragX, listener.dragY);
        var dragIndex = gridView.indexAt(dragPos.x, dragPos.y);

        if (listener.dragX == -1 || dragIndex != dropIndex) {
            dir.drop(target, event, dropItemAt(dropPos));
        }
    }

    function makeUpButton() {
        return Qt.createQmlObject("UpButtonItem {}", main);
    }

    Connections {
        target: root

        onIsPopupChanged: {
            if (upButton == null && root.isPopup) {
                upButton = makeUpButton();
            } else if (upButton != null) {
                upButton.destroy();
            }
        }
    }

    MouseEventListener {
        id: listener

        anchors {
            topMargin: upButton != null ? upButton.height : undefined
            fill: parent
        }

        property alias hoveredItem: gridView.hoveredItem

        property Item pressedItem: null
        property int pressX: -1
        property int pressY: -1
        property int dragX: -1
        property int dragY: -1
        property variant cPress: null
        property bool doubleClickInProgress: false

        acceptedButtons: {
            if (hoveredItem == null && main.isRootView) {
                return root.isPopup ? (Qt.LeftButton | Qt.MiddleButton) : Qt.LeftButton;
            }

            return root.isPopup ? (Qt.LeftButton | Qt.MiddleButton | Qt.RightButton)
                : (Qt.LeftButton | Qt.RightButton);
        }

        hoverEnabled: true

        onPressXChanged: {
            cPress = mapToItem(gridView.contentItem, pressX, pressY);
        }

        onPressYChanged: {
            cPress = mapToItem(gridView.contentItem, pressX, pressY);
        }

        onPressed: {
            // Ignore press events outside the viewport (i.e. on scrollbars).
            if (!scrollArea.viewport.contains(Qt.point(mouse.x,mouse.y))) {
                return;
            }

            scrollArea.focus = true;

            if (childAt(mouse.x, mouse.y) != editor) {
                editor.targetItem = null;
            }

            pressX = mouse.x;
            pressY = mouse.y;

            if (!hoveredItem || hoveredItem.blank) {
                if (!gridView.ctrlPressed) {
                    dir.clearSelection();
                }

                if (mouse.buttons & Qt.RightButton) {
                    clearPressState();
                    dir.openContextMenu();
                }
            } else {
                pressedItem = hoveredItem;

                var pos = mapToItem(hoveredItem.actionsOverlay, mouse.x, mouse.y);

                if (!(pos.x <= hoveredItem.actionsOverlay.width && pos.y <= hoveredItem.actionsOverlay.height)) {
                    if (gridView.shiftPressed && gridView.currentIndex != -1) {
                        positioner.setRangeSelected(gridView.anchorIndex, hoveredItem.index);
                    } else {
                        // FIXME TODO: Clicking one item with others selected should deselect the others,
                        // which doesn't happen right now because initiating a drag after the press should
                        // still drag all of them: The deselect needs to happen on release instead so we
                        // can distinguish.
                        if (!gridView.ctrlPressed && !dir.isSelected(positioner.map(hoveredItem.index))) {
                            dir.clearSelection();
                        }

                        if (gridView.ctrlPressed) {
                            dir.toggleSelected(positioner.map(hoveredItem.index));
                        } else {
                            dir.setSelected(positioner.map(hoveredItem.index));
                        }
                    }

                    gridView.currentIndex = hoveredItem.index;

                    if (mouse.buttons & Qt.RightButton) {
                        clearPressState();
                        dir.openContextMenu();
                    }
                }
            }

            main.pressed();
        }

        onCanceled: pressCanceled()
        onReleased: pressCanceled()

        onClicked: {
            clearPressState();

            if (mouse.buttons & Qt.RightButton) {
                return;
            }

            if (!hoveredItem || hoveredItem.blank || gridView.currentIndex == -1 || gridView.ctrlPressed || gridView.shiftPressed) {
                return;
            }

            var pos = mapToItem(hoveredItem.actionsOverlay, mouse.x, mouse.y);

            if (!(pos.x <= hoveredItem.actionsOverlay.width && pos.y <= hoveredItem.actionsOverlay.height)) {
                if (systemSettings.singleClick() || doubleClickInProgress) {
                    var func = root.isPopup && (mouse.button == Qt.LeftButton) && hoveredItem.isDir ? dir.cd : dir.run;
                    func(positioner.map(gridView.currentIndex));

                    hoveredItem = null;
                } else {
                    doubleClickInProgress = true;
                    doubleClickTimer.interval = systemSettings.doubleClickInterval();
                    doubleClickTimer.start();
                }
            }
        }

        onPositionChanged: {
            gridView.ctrlPressed = (mouse.modifiers & Qt.ControlModifier);
            gridView.shiftPressed = (mouse.modifiers & Qt.ShiftModifier);

            var cPos = mapToItem(gridView.contentItem, mouse.x, mouse.y);
            var item = gridView.itemAt(cPos.x, cPos.y);
            var leftEdge = Math.min(gridView.contentX, gridView.originX);

            if (!item || item.blank) {
                gridView.hoveredItem = null;
            } else {
                var aPos = mapToItem(item.actionsOverlay, mouse.x, mouse.y);
                var hPos = mapToItem(item.hoverArea, mouse.x, mouse.y);

                if ((hPos.x < 0 || hPos.y < 0 || hPos.x > item.hoverArea.width || hPos.y > item.hoverArea.height)
                    && aPos.x < 0) {
                    gridView.hoveredItem = null;
                }
            }

            // Trigger autoscroll.
            if (pressX != -1) {
                gridView.scrollLeft = (mouse.x <= 0 && gridView.contentX > leftEdge);
                gridView.scrollRight = (mouse.x >= gridView.width
                    && gridView.contentX < gridView.contentItem.width - gridView.width);
                gridView.scrollUp = (mouse.y <= 0 && gridView.contentY > 0);
                gridView.scrollDown = (mouse.y >= gridView.height
                    && gridView.contentY < gridView.contentItem.height - gridView.height);
            }

            // Update rubberband geometry.
            if (main.rubberBand) {
                var rB = main.rubberBand;

                if (cPos.x < cPress.x) {
                    rB.x = Math.max(leftEdge, cPos.x);
                    rB.width = Math.abs(rB.x - cPress.x);
                } else {
                    rB.x = cPress.x;
                    var ceil = Math.max(gridView.width, gridView.contentItem.width) + leftEdge;
                    rB.width = Math.min(ceil - rB.x, Math.abs(rB.x - cPos.x));
                }

                if (cPos.y < cPress.y) {
                    rB.y = Math.max(0, cPos.y);
                    rB.height = Math.abs(rB.y - cPress.y);
                } else {
                    rB.y = cPress.y;
                    var ceil = Math.max(gridView.height, gridView.contentItem.height);
                    rB.height = Math.min(ceil - rB.y, Math.abs(rB.y - cPos.y));
                }

                gridView.rectangleSelect(rB.x, rB.y, rB.width, rB.height);

                return;
            }

            // Drag initiation.
            if (pressX != -1 && systemSettings.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                if (pressedItem != null && dir.isSelected(positioner.map(pressedItem.index))) {
                    dragX = mouse.x;
                    dragY = mouse.y;
                    dir.dragSelected(mouse.x, mouse.y);
                    dragX = -1;
                    dragY = -1;
                    clearPressState();
                } else {
                    // Disable rubberband in popup list view mode.
                    if (root.isPopup) {
                        return;
                    }

                    dir.pinSelection();
                    main.rubberBand = Qt.createQmlObject("import QtQuick 2.0; import org.kde.private.desktopcontainment.folder 0.1 as Folder;"
                        + "Folder.RubberBand { x: " + cPress.x + "; y: " + cPress.y + "; width: 0; height: 0; z: 99999; }",
                        gridView.contentItem);
                    gridView.interactive = false;
                }
            }
        }

        onContainsMouseChanged: {
            if (!containsMouse && !main.rubberBand) {
                clearPressState();
            }
        }

        onHoveredItemChanged: {
            doubleClickInProgress = false;
        }

        function pressCanceled() {
            if (main.rubberBand) {
                main.rubberBand.visible = false;
                main.rubberBand.enabled = false;
                main.rubberBand.destroy();
                main.rubberBand = null;
                gridView.interactive = true;
                gridView.cachedRectangleSelection = null;
                dir.unpinSelection();
            }

            clearPressState();
            gridView.cancelAutoscroll();
        }

        function clearPressState() {
            pressedItem = null;
            pressX = -1;
            pressY = -1;
        }

        Timer {
            id: doubleClickTimer

            onTriggered: {
                listener.doubleClickInProgress = false;
            }
        }

        PlasmaExtras.ScrollArea {
            id: scrollArea

            anchors.fill: parent

            focus: true

            GridView {
                id: gridView

                property bool isRootView: false

                property int iconSize: makeIconSize()

                property Item hoveredItem: null

                property int anchorIndex: 0
                property bool ctrlPressed: false
                property bool shiftPressed: false

                property bool overflowing: (visibleArea.heightRatio < 1.0 || visibleArea.widthRatio < 1.0)

                property bool scrollLeft: false
                property bool scrollRight: false
                property bool scrollUp: false
                property bool scrollDown: false

                property variant cachedRectangleSelection: null

                currentIndex: -1

                keyNavigationWraps: false
                boundsBehavior: Flickable.StopAtBounds

                cellWidth: {
                    if (root.isPopup) {
                        return gridView.width;
                    }

                    return iconSize + (4 * units.largeSpacing);
                }

                cellHeight: {
                    if (root.isPopup) {
                        return Math.ceil((Math.max(theme.mSize(theme.defaultFont).height, iconSize)
                            + Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
                            listItemSvg.margins.top + listItemSvg.margins.bottom)) / 2) * 2;
                    }

                    return (iconSize + (theme.mSize(theme.defaultFont).height * plasmoid.configuration.textLines)
                        + (3 * units.smallSpacing) + (2 * units.largeSpacing));
                }

                model: positioner

                delegate: FolderItemDelegate {
                    width: gridView.cellWidth
                    height: gridView.cellHeight
                }

                onContentXChanged: {
                    dir.setDragHotSpotScrollOffset(contentX, contentY);

                    if (contentX == 0) {
                        scrollLeft = false;
                    }

                    if (contentX == contentItem.width - width) {
                        scrollRight = false;
                    }

                    // Update rubberband geomety.
                    if (main.rubberBand) {
                        var rB = main.rubberBand;

                        if (scrollLeft) {
                            rB.x = Math.min(gridView.contentX, gridView.originX);
                            rB.width = listener.cPress.x;
                        }

                        if (scrollRight) {
                            var lastCol = gridView.contentX + gridView.width;
                            rB.width = lastCol - rB.x;
                        }

                        gridView.rectangleSelect(rB.x, rB.y, rB.width, rB.height);
                    }
                }

                onContentYChanged: {
                    dir.setDragHotSpotScrollOffset(contentX, contentY);

                    if (contentY == 0) {
                        scrollUp = false;
                    }

                    if (contentY == contentItem.height - height) {
                        scrollDown = false;
                    }

                    // Update rubberband geomety.
                    if (main.rubberBand) {
                        var rB = main.rubberBand;

                        if (scrollUp) {
                            rB.y = 0;
                            rB.height = listener.cPress.y;
                        }

                        if (scrollDown) {
                            var lastRow = gridView.contentY + gridView.height;
                            rB.height = lastRow - rB.y;
                        }

                        gridView.rectangleSelect(rB.x, rB.y, rB.width, rB.height);
                    }
                }

                onScrollLeftChanged: {
                    if (scrollLeft && gridView.visibleArea.widthRatio < 1.0) {
                        smoothX.enabled = true;
                        contentX = (gridView.flow == GridView.FlowLeftToRight) ? gridView.contentX : gridView.originX;
                    } else {
                        contentX = contentX;
                        smoothX.enabled = false;
                    }
                }

                onScrollRightChanged: {
                    if (scrollRight && gridView.visibleArea.widthRatio < 1.0) {
                        smoothX.enabled = true;
                        contentX = ((gridView.flow == GridView.FlowLeftToRight) ? gridView.contentX : gridView.originX)
                            + (contentItem.width - width);
                    } else {
                        contentX = contentX;
                        smoothX.enabled = false;
                    }
                }

                onScrollUpChanged: {
                    if (scrollUp && gridView.visibleArea.heightRatio < 1.0) {
                        smoothY.enabled = true;
                        contentY = 0;
                    } else {
                        contentY = contentY;
                        smoothY.enabled = false;
                    }
                }

                onScrollDownChanged: {
                    if (scrollDown && gridView.visibleArea.heightRatio < 1.0) {
                        smoothY.enabled = true;
                        contentY = contentItem.height - height;
                    } else {
                        contentY = contentY;
                        smoothY.enabled = false;
                    }
                }

                onFlowChanged: {
                    // FIXME TODO: Preserve positions.
                    if (positioner.enabled) {
                        positioner.reset();
                    }
                }

                onLayoutDirectionChanged: {
                    // FIXME TODO: Preserve positions.
                    if (positioner.enabled) {
                        positioner.reset();
                    }
                }

                onCurrentIndexChanged: {
                    positionViewAtIndex(currentIndex, GridView.Contain);
                }

                onCachedRectangleSelectionChanged: {
                    if (cachedRectangleSelection) {
                        dir.updateSelection(cachedRectangleSelection, gridView.ctrlPressed);
                    }
                }

                function makeIconSize() {
                    if (root.isPopup) {
                        return units.iconSizes.small;
                    }

                    return FolderTools.iconSizeFromTheme(plasmoid.configuration.iconSize);
                }

                function updateSelection(modifier) {
                    if (modifier & Qt.ShiftModifier) {
                        positioner.setRangeSelected(anchorIndex, currentIndex);
                    } else {
                        dir.clearSelection();
                        dir.setSelected(positioner.map(currentIndex));
                    }
                }

                function cancelAutoscroll() {
                    scrollLeft = false;
                    scrollRight = false;
                    scrollUp = false;
                    scrollDown = false;
                }

                function rectangleSelect(x, y, width, height) {
                    var rows = (gridView.flow == GridView.FlowLeftToRight);
                    var axis = rows ? gridView.width : gridView.height;
                    var step = rows ? cellWidth : cellHeight;
                    var perStripe = Math.floor(axis / step);
                    var stripes = Math.ceil(gridView.count / perStripe);
                    var cWidth = gridView.cellWidth - (2 * units.smallSpacing);
                    var cHeight = gridView.cellHeight - (2 * units.smallSpacing);
                    var midWidth = gridView.cellWidth / 2;
                    var midHeight = gridView.cellHeight / 2;
                    var indices = [];

                    for (var s = 0; s < stripes; s++) {
                        for (var i = 0; i < perStripe; i++) {
                            var index = (s * perStripe) + i;

                            if (index >= gridView.count) {
                                break;
                            }

                            if (positioner.isBlank(index)) {
                                continue;
                            }

                            var itemX = ((rows ? i : s) * gridView.cellWidth);
                            var itemY = ((rows ? s : i) * gridView.cellHeight);

                            if (gridView.layoutDirection == Qt.RightToLeft) {
                                itemX -= (rows ? gridView.contentX : gridView.originX);
                                itemX += cWidth;
                                itemX = (rows ? gridView.width : gridView.contentItem.width) - itemX;
                            }

                            // Check if the rubberband intersects this cell first to avoid doing more
                            // expensive work.
                            if (main.rubberBand.intersects(Qt.rect(itemX + units.smallSpacing, itemY + units.smallSpacing,
                                cWidth, cHeight))) {
                                var item = gridView.contentItem.childAt(itemX + midWidth, itemY + midHeight);

                                // If this is a visible item, check for intersection with the actual
                                // icon or label rects for better feel.
                                if (item && item.iconArea) {
                                    var iconRect = Qt.rect(itemX + item.iconArea.x, itemY + item.iconArea.y,
                                        item.iconArea.width, item.iconArea.height);

                                    if (main.rubberBand.intersects(iconRect)) {
                                        indices.push(positioner.map(index));
                                        continue;
                                    }

                                    var labelRect = Qt.rect(itemX + item.labelArea.x, itemY + item.labelArea.y,
                                        item.labelArea.width, item.labelArea.height);

                                    if (main.rubberBand.intersects(labelRect)) {
                                        indices.push(positioner.map(index));
                                        continue;
                                    }
                                } else {
                                    // Otherwise be content with the cell intersection.
                                    indices.push(positioner.map(index));
                                }
                            }
                        }
                    }

                    gridView.cachedRectangleSelection = indices;
                }

                Behavior on contentX { id: smoothX; enabled: false; SmoothedAnimation { velocity: 700 } }
                Behavior on contentY { id: smoothY; enabled: false; SmoothedAnimation { velocity: 700 } }

                Keys.onReturnPressed: {
                    if (currentIndex != -1) {
                        var func = root.isPopup ? dir.cd : dir.run;
                        func(positioner.map(currentIndex));
                    }
                }

                Keys.onMenuPressed: {
                    // FIXME TODO: Correct popup position.
                    return;

                    if (currentIndex != -1) {
                        dir.setSelected(positioner.map(currentIndex));
                        dir.openContextMenu();
                    }
                }

                Keys.onPressed: {
                    if (event.matches(StandardKey.Delete)) {
                        if (dir.hasSelection()) {
                            dir.action("trash").trigger();
                        }
                    } else if (event.key == Qt.Key_Control) {
                        ctrlPressed = true;
                    } else if (event.key == Qt.Key_Shift) {
                        shiftPressed = true;

                        if (currentIndex != -1) {
                            anchorIndex = currentIndex;
                        }
                    } else if (event.key == Qt.Key_Home) {
                        currentIndex = 0;
                        updateSelection(event.modifiers);
                    } else if (event.key == Qt.Key_End) {
                        currentIndex = count - 1;
                        updateSelection(event.modifiers);
                    }
                }

                Keys.onReleased: {
                    if (event.key == Qt.Key_Control) {
                        ctrlPressed = false;
                    } else if (event.key == Qt.Key_Shift) {
                        shiftPressed = false;
                        anchorIndex = 0;
                    }
                }

                Keys.onLeftPressed: {
                    if (positioner.enabled) {
                        var newIndex = positioner.nearestItem(currentIndex,
                            FolderTools.effectiveNavDirection(gridView.flow, gridView.layoutDirection, Qt.LeftArrow));

                        if (newIndex != -1) {
                            currentIndex = newIndex;
                            updateSelection(event.modifiers);
                        }
                    } else {
                        var oldIndex = currentIndex;

                        moveCurrentIndexLeft();

                        if (oldIndex == currentIndex) {
                            return;
                        }

                        updateSelection(event.modifiers);
                    }
                }

                Keys.onRightPressed: {
                    if (positioner.enabled) {
                        var newIndex = positioner.nearestItem(currentIndex,
                            FolderTools.effectiveNavDirection(gridView.flow, gridView.layoutDirection, Qt.RightArrow));

                        if (newIndex != -1) {
                            currentIndex = newIndex;
                            updateSelection(event.modifiers);
                        }
                    } else {
                        var oldIndex = currentIndex;

                        moveCurrentIndexRight();

                        if (oldIndex == currentIndex) {
                            return;
                        }

                        updateSelection(event.modifiers);
                    }
                }

                Keys.onUpPressed: {
                    if (positioner.enabled) {
                        var newIndex = positioner.nearestItem(currentIndex,
                            FolderTools.effectiveNavDirection(gridView.flow, gridView.layoutDirection, Qt.UpArrow));

                        if (newIndex != -1) {
                            currentIndex = newIndex;
                            updateSelection(event.modifiers);
                        }
                    } else {
                        var oldIndex = currentIndex;

                        moveCurrentIndexUp();

                        if (oldIndex == currentIndex) {
                            return;
                        }

                        updateSelection(event.modifiers);
                    }
                }

                Keys.onDownPressed: {
                    if (positioner.enabled) {
                        var newIndex = positioner.nearestItem(currentIndex,
                            FolderTools.effectiveNavDirection(gridView.flow, gridView.layoutDirection, Qt.DownArrow));

                        if (newIndex != -1) {
                            currentIndex = newIndex;
                            updateSelection(event.modifiers);
                        }
                    } else {
                        var oldIndex = currentIndex;

                        moveCurrentIndexDown();

                        if (oldIndex == currentIndex) {
                            return;
                        }

                        updateSelection(event.modifiers);
                    }
                }

                Connections {
                    target: units

                    onIconSizesChanged: {
                        gridView.iconSize = gridView.makeIconSize();
                    }
                }

                Connections {
                    target: plasmoid.configuration

                    onIconSizeChanged: {
                        gridView.iconSize = gridView.makeIconSize();
                    }
                }
            }
        }

        Folder.WheelInterceptor {
            anchors.fill: parent

            enabled: root.isContainment && !gridView.overflowing
            destination: plasmoid
        }

        Folder.FolderModel {
            id: dir

            usedByContainment: root.isContainment && main.isRootView
            sortDesc: plasmoid.configuration.sortDesc
            sortDirsFirst: plasmoid.configuration.sortDirsFirst
            parseDesktopFiles: (url == "desktop:/")
            previews: plasmoid.configuration.previews
            previewPlugins: plasmoid.configuration.previewPlugins

            onMove: {
                var rows = (gridView.flow == GridView.FlowLeftToRight);
                var axis = rows ? gridView.width : gridView.height;
                var step = rows ? cellWidth : cellHeight;
                var perStripe = Math.floor(axis / step);
                var dropPos = mapToItem(gridView.contentItem, x, y);
                var leftEdge = Math.min(gridView.contentX, gridView.originX);

                var moves = []
                var itemX = -1;
                var itemY = -1;
                var col = -1;
                var row = -1;
                var from = -1;
                var to = -1;

                for (var i = 0; i < urls.length; i++) {
                    from = positioner.indexForUrl(urls[i]);
                    to = -1;

                    if (from == -1) {
                        continue;
                    }

                    var offset = dir.dragCursorOffset(positioner.map(from));

                    if (offset.x == -1) {
                        continue;
                    }

                    itemX = dropPos.x + offset.x + (listener.dragX % cellWidth);
                    itemY = dropPos.y + offset.y + (listener.dragY % cellHeight);

                    if (gridView.layoutDirection == Qt.RightToLeft) {
                        itemX -= (rows ? gridView.contentX : gridView.originX);
                        itemX = (rows ? gridView.width : gridView.contentItem.width) - itemX;
                    }

                    col = Math.floor(itemX / gridView.cellWidth);
                    row = Math.floor(itemY / gridView.cellHeight);

                    if ((rows ? col : row) < perStripe) {
                        to = ((rows ? row : col) * perStripe) + (rows ? col : row);

                        if (to < 0) {
                            return;
                        }
                    }

                    if (from != to) {
                        moves.push(from);
                        moves.push(to);
                    }
                }

                if (moves.length) {
                    positioner.move(moves);
                    gridView.forceLayout();
                }

                dir.clearSelection();
            }
        }

        Folder.Positioner {
            id: positioner

            enabled: (isContainment && dir.sortMode == -1)

            folderModel: dir

            perStripe: Math.floor(((gridView.flow == GridView.FlowLeftToRight)
                ? gridView.width : gridView.height) / ((gridView.flow == GridView.FlowLeftToRight)
                ? gridView.cellWidth : gridView.cellHeight));
        }

        Folder.ItemViewAdapter {
            id: viewAdapter

            adapterView: gridView
            adapterModel: positioner
            adapterIconSize: gridView.iconSize;
            adapterVisibleArea: Qt.rect(gridView.contentX, gridView.contentY, gridView.contentWidth, gridView.contentHeight)

            Component.onCompleted: {
                gridView.movementStarted.connect(viewAdapter.viewScrolled);
                dir.viewAdapter = viewAdapter;
            }
        }

        function rename()
        {
            if (gridView.currentIndex != -1) {
                editor.targetItem = gridView.currentItem;
            }
        }

        PlasmaComponents.TextField {
            id: editor

            visible: false

            property Item targetItem: null

            onTargetItemChanged: {
                if (targetItem != null) {
                    var pos = main.mapFromItem(targetItem, targetItem.labelArea.x, targetItem.labelArea.y);
                    x = pos.x + (units.smallSpacing * 2);
                    y = pos.y + (units.smallSpacing * 2);
                    width = targetItem.labelArea.width;
                    height = targetItem.labelArea.height;
                    text = targetItem.label.text;
                    editor.selectAll();
                    visible = true;
                } else {
                    x: 0
                    y: 0
                    visible = false;
                }
            }

            onVisibleChanged: {
                if (visible) {
                    focus = true;
                } else {
                    scrollArea.focus = true;
                }
            }

            onAccepted: {
                dir.rename(positioner.map(targetItem.index), text);
                targetItem = null;
            }
        }

        Component.onCompleted: {
            dir.requestRename.connect(rename);
        }
    }

    Component.onCompleted: {
        if (upButton == null && root.isPopup) {
            upButton = makeUpButton();
        }
    }
}
