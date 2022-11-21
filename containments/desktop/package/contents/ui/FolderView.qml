/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQml 2.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PC3
import org.kde.kquickcontrolsaddons 2.0

import org.kde.private.desktopcontainment.folder 0.1 as Folder
import "code/FolderTools.js" as FolderTools

FocusScope {
    id: main

    signal pressed()

    property QtObject model: dir
    property Item rubberBand: null

    property alias view: gridView
    property alias isRootView: gridView.isRootView
    property alias currentIndex: gridView.currentIndex
    property alias url: dir.url
    property alias status: dir.status
    property alias perStripe: positioner.perStripe
    property alias positions: positioner.positions
    property alias errorString: dir.errorString
    property alias dragging: dir.dragging
    property alias dragInProgressAnywhere: dir.dragInProgressAnywhere
    property alias locked: dir.locked
    property alias sortMode: dir.sortMode
    property alias filterMode: dir.filterMode
    property alias filterPattern: dir.filterPattern
    property alias filterMimeTypes: dir.filterMimeTypes
    property alias showHiddenFiles: dir.showHiddenFiles
    property alias flow: gridView.flow
    property alias layoutDirection: gridView.layoutDirection
    property alias cellWidth: gridView.cellWidth
    property alias cellHeight: gridView.cellHeight
    property alias overflowing: gridView.overflowing
    property alias scrollLeft: gridView.scrollLeft
    property alias scrollRight: gridView.scrollRight
    property alias scrollUp: gridView.scrollUp
    property alias scrollDown: gridView.scrollDown
    property alias hoveredItem: gridView.hoveredItem
    property var history: []
    property var lastPosition: null
    property bool goingBack: false
    property Item backButton: null
    property var dialog: null
    property Item editor: null

    property int previouslySelectedItemIndex: -1

    function positionViewAtBeginning() {
        gridView.positionViewAtBeginning();
    }

    function rename() {
        if (gridView.currentIndex !== -1) {
            var renameAction = folderView.model.action("rename");
            if (renameAction && !renameAction.enabled) {
                return;
            }

            if (!editor) {
                editor = editorComponent.createObject(listener);
            }

            editor.targetItem = gridView.currentItem;
        }
    }

    function cancelRename() {
        if (editor) {
            editor.targetItem = null;
        }
    }

    function linkHere(sourceUrl) {
        dir.linkHere(sourceUrl);
    }

    function handleDragMove(x, y) {
        var child = childAt(x, y);

        if (child !== null && child === backButton) {
            hoveredItem = null;
            backButton.handleDragMove();
        } else {
            if (backButton && backButton.containsDrag) {
                backButton.endDragMove();
            }

            var pos = mapToItem(gridView.contentItem, x, y);
            var item = gridView.itemAt(pos.x, pos.y);

            if (item && item.isDir) {
                hoveredItem = item;
            } else {
                hoveredItem = null;
            }
        }
    }

    function endDragMove() {
        if (backButton && backButton.active) {
            backButton.endDragMove();
        } else if (hoveredItem && !hoveredItem.popupDialog) {
            hoveredItem = null;
        }
    }

    function dropItemAt(pos) {
        var item = gridView.itemAt(pos.x, pos.y);

        if (item) {
            if (item.blank) {
                return -1;
            }

            var hOffset = Math.abs(Math.min(gridView.contentX, gridView.originX));
            var hPos = mapToItem(item.hoverArea, pos.x + hOffset, pos.y);

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

        if (listener.dragX === -1 || dragIndex !== dropIndex) {
            dir.drop(target, event, dropItemAt(dropPos), root.isContainment && !plasmoid.immutable);
        }
    }

    Connections {
        target: dir
        function onPopupMenuAboutToShow(dropJob, mimeData, x, y) {
            if (root.isContainment && !plasmoid.immutable) {
                plasmoid.processMimeData(mimeData, x, y, dropJob);
            }
        }
    }

    Connections {
        target: plasmoid
        function onExpandedChanged() {
            if (plasmoid.expanded && dir.status === Folder.FolderModel.Ready && !gridView.model) {
                gridView.model = positioner;
            }
        }
    }

    Binding {
        target: plasmoid
        property: "busy"
        value: !gridView.model && dir.status === Folder.FolderModel.Listing
        restoreMode: Binding.RestoreBinding
    }

    function makeBackButton() {
        return Qt.createQmlObject("BackButtonItem {}", main);
    }

    function doCd(row) {
        history.push({ url: url, index: gridView.currentIndex, yPosition: gridView.visibleArea.yPosition});
        updateHistory();
        dir.cd(row);
        gridView.currentIndex = -1;
    }

    function doBack() {
        goingBack = true;
        gridView.currentIndex = -1;
        lastPosition = history.pop();
        url = lastPosition.url;
        updateHistory();
    }

    // QML doesn't detect change in the array(history) property, so update it explicitly.
    function updateHistory() {
        history = history;
    }

    Connections {
        target: root

        function onIsPopupChanged() {
            if (backButton === null && root.useListViewMode) {
                backButton = makeBackButton();
            } else if (backButton !== null) {
                backButton.destroy();
            }
        }
    }

    MouseEventListener {
        id: listener

        anchors {
            topMargin: backButton !== null ? backButton.height : undefined
            fill: parent
        }

        property alias hoveredItem: gridView.hoveredItem

        property Item pressedItem: null
        property int pressX: -1
        property int pressY: -1
        property int dragX: -1
        property int dragY: -1
        property var cPress: null
        property bool doubleClickInProgress: false

        acceptedButtons: {
            if (hoveredItem === null && main.isRootView) {
                return root.isPopup ? (Qt.LeftButton | Qt.MiddleButton | Qt.BackButton) : Qt.LeftButton;
            }

            return root.isPopup ? (Qt.LeftButton | Qt.MiddleButton | Qt.RightButton | Qt.BackButton)
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
            if (!scrollArea.viewport.contains(Qt.point(mouse.x, mouse.y))) {
                return;
            }

            scrollArea.focus = true;

            if (mouse.buttons & Qt.BackButton) {
                if (root.isPopup && dir.resolvedUrl !== dir.resolve(plasmoid.configuration.url)) {
                    doBack();
                    mouse.accepted = true;
                }

                return;
            }

            if (editor && childAt(mouse.x, mouse.y) !== editor) {
                editor.commit();
            }

            if (mouse.source === Qt.MouseEventSynthesizedByQt) {
                var index = gridView.indexAt(mouse.x, mouse.y);
                var indexItem = gridView.itemAtIndex(index);
                if (indexItem && indexItem.iconArea) {
                    gridView.currentIndex = index;
                    hoveredItem = indexItem;
                } else {
                    hoveredItem = null;
                }
                if (gridView.hoveredItem && gridView.hoveredItem.toolTip.active) {
                    gridView.hoveredItem.toolTip.hideToolTip();
                }
            }

            pressX = mouse.x;
            pressY = mouse.y;

            if (!hoveredItem || hoveredItem.blank) {
                if (!gridView.ctrlPressed) {
                    gridView.currentIndex = -1;
                    previouslySelectedItemIndex = -1;
                    dir.clearSelection();
                }

                if (mouse.buttons & Qt.RightButton) {
                    clearPressState();
                    dir.openContextMenu(main, mouse.modifiers);
                    mouse.accepted = true;
                }
            } else {
                pressedItem = hoveredItem;

                var pos = mapToItem(hoveredItem.actionsOverlay, mouse.x, mouse.y);

                if (!(pos.x <= hoveredItem.actionsOverlay.width && pos.y <= hoveredItem.actionsOverlay.height)) {
                    if (gridView.shiftPressed && gridView.currentIndex !== -1) {
                        positioner.setRangeSelected(gridView.anchorIndex, hoveredItem.index);
                    } else {
                        // Deselecting everything else when one item is clicked is handled in onReleased in order to distinguish between drag and click
                        if (!gridView.ctrlPressed && !dir.isSelected(positioner.map(hoveredItem.index))) {
                            previouslySelectedItemIndex = -1;
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
                        if (pressedItem.toolTip && pressedItem.toolTip.active) {
                            pressedItem.toolTip.hideToolTip();
                        }

                        clearPressState();

                        dir.openContextMenu(hoveredItem, mouse.modifiers);
                        mouse.accepted = true;
                    }
                }
            }

            main.pressed();
        }

        onCanceled: pressCanceled()

        onReleased: {
            if (hoveredItem && !hoveredItem.blank && mouse.button !== Qt.RightButton) {
                var pos = mapToItem(hoveredItem.actionsOverlay, mouse.x, mouse.y);
                if (!(pos.x <= hoveredItem.actionsOverlay.width && pos.y <= hoveredItem.actionsOverlay.height)
                    && (!(gridView.shiftPressed && gridView.currentIndex !== -1) && !gridView.ctrlPressed)) {
                        dir.clearSelection();
                        dir.setSelected(positioner.map(hoveredItem.index));
                }
            }
            pressCanceled();
        }

        onPressAndHold: {
            if (mouse.source === Qt.MouseEventSynthesizedByQt) {
                if (pressedItem) {
                    if (pressedItem.toolTip && pressedItem.toolTip.active) {
                        pressedItem.toolTip.hideToolTip();
                    }
                }
                clearPressState();
                if (hoveredItem) {
                    dir.openContextMenu(hoveredItem, mouse.modifiers);
                }
            }
        }

        onClicked: {
            clearPressState();

            if (mouse.button === Qt.RightButton ||
                (editor && childAt(mouse.x, mouse.y) === editor)) {
                return;
            }

            if (!hoveredItem || hoveredItem.blank || gridView.currentIndex === -1 || gridView.ctrlPressed || gridView.shiftPressed) {
                // Bug 357367: Replay mouse event, so containment actions assigned to left mouse button work.
                eventGenerator.sendMouseEvent(plasmoid, EventGenerator.MouseButtonPress, mouse.x, mouse.y, mouse.button, mouse.buttons, mouse.modifiers);
                return;
            }

            var pos = mapToItem(hoveredItem, mouse.x, mouse.y);

            // Moving from an item to its preview popup dialog doesn't unset hoveredItem
            // even though the cursor has left it, so we need to check whether the click
            // actually occurred inside the item we expect it in before going ahead. If it
            // didn't, clean up (e.g. dismissing the dialog as a side-effect of unsetting
            // hoveredItem) and abort.
            if (pos.x < 0 || pos.x > hoveredItem.width || pos.y < 0 || pos.y > hoveredItem.height) {
                hoveredItem = null;
                previouslySelectedItemIndex = -1;
                dir.clearSelection();

                return;
            // If the hoveredItem is clicked while having a preview popup dialog open,
            // only dismiss the dialog and abort.
            } else if (hoveredItem.popupDialog) {
                hoveredItem.closePopup();

                return;
            }

            pos = mapToItem(hoveredItem.actionsOverlay, mouse.x, mouse.y);

            if (!(pos.x <= hoveredItem.actionsOverlay.width && pos.y <= hoveredItem.actionsOverlay.height)) {

                // Clicked on the label of an already-selected item: rename it
                if (pos.x > hoveredItem.labelArea.x
                    && pos.x <= hoveredItem.labelArea.x + hoveredItem.labelArea.width
                    && pos.y > hoveredItem.labelArea.y
                    && pos.y <= hoveredItem.labelArea.y + hoveredItem.labelArea.height
                    && previouslySelectedItemIndex === gridView.currentIndex
                    && gridView.currentIndex !== -1
                    && !Qt.styleHints.singleClickActivation
                    && plasmoid.configuration.renameInline
                    && !doubleClickInProgress
                ) {
                    rename();
                    return;
                }

                // Single-click mode and single-clicked on the item or
                // double-click mode and double-clicked on the item: open it
                if (Qt.styleHints.singleClickActivation || doubleClickInProgress || mouse.source === Qt.MouseEventSynthesizedByQt) {
                    var func = root.useListViewMode && (mouse.button === Qt.LeftButton) && hoveredItem.isDir ? doCd : dir.run;
                    func(positioner.map(gridView.currentIndex));
                    previouslySelectedItemIndex = gridView.currentIndex;
                    hoveredItem = null;
                } else {
                    // None of the above: select it
                    doubleClickInProgress = true;
                    doubleClickTimer.interval = Qt.styleHints.mouseDoubleClickInterval;
                    doubleClickTimer.start();
                    previouslySelectedItemIndex = gridView.currentIndex;
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
                if (gridView.hoveredItem && !root.containsDrag && (!dialog || !dialog.containsDrag) && !gridView.hoveredItem.popupDialog) {
                    gridView.hoveredItem = null;
                }
            } else {
                var fPos = mapToItem(item.frame, mouse.x, mouse.y);

                if (fPos.x < 0 || fPos.y < 0 || fPos.x > item.frame.width || fPos.y > item.frame.height) {
                    gridView.hoveredItem = null;
                }
            }

            // Trigger autoscroll.
            if (pressX !== -1) {
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

                // Ensure rubberband is at least 1px in size or else it will become
                // invisible and not match any items.
                rB.width = Math.max(1, rB.width);
                rB.height = Math.max(1, rB.height);

                gridView.rectangleSelect(rB.x, rB.y, rB.width, rB.height);

                return;
            }

            // Drag initiation.
            if (pressX !== -1 && root.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                if (pressedItem !== null && dir.isSelected(positioner.map(pressedItem.index))) {
                    pressedItem.toolTip.hideToolTip();
                    dragX = mouse.x;
                    dragY = mouse.y;
                    gridView.verticalDropHitscanOffset = pressedItem.iconArea.y + (pressedItem.iconArea.height / 2);
                    dir.dragSelected(mouse.x, mouse.y);
                    dragX = -1;
                    dragY = -1;
                    clearPressState();
                } else {
                    // Disable rubberband in popup list view mode or while renaming
                    if (root.useListViewMode || (editor && editor.targetItem)) {
                        return;
                    }

                    dir.pinSelection();
                    main.rubberBand = rubberBandObject.createObject(gridView.contentItem, {x: cPress.x, y: cPress.y});
                    gridView.interactive = false;
                }
            }
        }

        Component {
            id: rubberBandObject

            Folder.RubberBand {
                id: rubberBand

                width: 0
                height: 0
                z: 99999

                function close() {
                    opacityAnimation.restart();
                }

                OpacityAnimator {
                    id: opacityAnimation
                    target: rubberBand
                    to: 0
                    from: 1
                    duration: PlasmaCore.Units.shortDuration

                    // This easing curve has an elognated start, which works
                    // better than a standard easing curve for the rubberband
                    // animation, which fades out fast and is generally of a
                    // small area.
                    easing {
                        bezierCurve: [0.4, 0.0, 1, 1]
                        type: Easing.Bezier
                    }

                    onFinished: {
                        rubberBand.visible = false;
                        rubberBand.enabled = false;
                        rubberBand.destroy();
                    }
                }
            }
        }

        onContainsMouseChanged: {
            if (!containsMouse && !main.rubberBand) {
                clearPressState();

                if (gridView.hoveredItem && !gridView.hoveredItem.popupDialog) {
                    gridView.hoveredItem = null;
                }
            }
        }

        onHoveredItemChanged: {
            doubleClickInProgress = false;

            if (!hoveredItem) {
                hoverActivateTimer.stop();
            }
        }

        function pressCanceled() {
            if (main.rubberBand) {
                main.rubberBand.close();
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

        Timer {
            id: hoverActivateTimer

            interval: root.hoverActivateDelay

            onTriggered: {
                if (!hoveredItem) {
                    return;
                }

                if (root.useListViewMode) {
                    doCd(index);
                } else if (plasmoid.configuration.popups) {
                    hoveredItem.openPopup();
                }
            }
        }

        FocusScope {
            id: scrollArea

            anchors.fill: parent

            focus: true

            property bool ready: false
            readonly property int viewportWidth: scrollArea.ready && viewport ? Math.ceil(viewport.width) : 0
            readonly property int viewportHeight: scrollArea.ready && viewport ? Math.ceil(viewport.height) : 0
            readonly property Flickable viewport: gridView

            Component.onCompleted: {
                scrollArea.ready = true;
            }

            GridView {
                id: gridView
                clip: true
                anchors.fill: parent

                property bool isRootView: false

                property int iconSize: makeIconSize()
                property int verticalDropHitscanOffset: 0

                property Item hoveredItem: null

                property int anchorIndex: 0
                property bool ctrlPressed: false
                property bool shiftPressed: false

                property bool overflowing: {
                    // widthRatio or heightRatio may be 0 when it's not actually
                    // overflowing, so account for that.
                    let widthOverflow =  visibleArea.widthRatio > 0.0 && visibleArea.widthRatio < 1.0
                    let heightOverflow = visibleArea.heightRatio > 0.0 && visibleArea.heightRatio < 1.0
                    return widthOverflow || heightOverflow
                }

                property bool scrollLeft: false
                property bool scrollRight: false
                property bool scrollUp: false
                property bool scrollDown: false

                property var cachedRectangleSelection: null

                currentIndex: -1

                keyNavigationWraps: false
                boundsBehavior: Flickable.StopAtBounds
                focus: true

                PC3.ScrollBar.vertical: PC3.ScrollBar {}
                PC3.ScrollBar.horizontal: PC3.ScrollBar {}

                function calcExtraSpacing(cellSize, containerSize) {
                    var availableColumns = Math.floor(containerSize / cellSize);
                    var extraSpacing = 0;
                    if (availableColumns > 0) {
                        var allColumnSize = availableColumns * cellSize;
                        var extraSpace = Math.max(containerSize - allColumnSize, 0);
                        extraSpacing = extraSpace / availableColumns;
                    }
                    return Math.floor(extraSpacing);
                }

                cellWidth: {
                    if (root.useListViewMode) {
                        return gridView.width;
                    } else {
                        var iconWidth = iconSize + (2 * PlasmaCore.Units.largeSpacing) + (2 * PlasmaCore.Units.smallSpacing);
                        if (root.isContainment && isRootView && scrollArea.viewportWidth > 0) {
                            var minIconWidth = Math.max(iconWidth, PlasmaCore.Units.iconSizes.small * ((plasmoid.configuration.labelWidth * 2) + 4));
                            var extraWidth = calcExtraSpacing(minIconWidth, scrollArea.viewportWidth);
                            return minIconWidth + extraWidth;
                        } else {
                            return iconWidth;
                        }
                    }
                }

                cellHeight: {
                    if (root.useListViewMode) {
                        return Math.ceil((Math.max(PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height, iconSize)
                            + Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
                            listItemSvg.margins.top + listItemSvg.margins.bottom)) / 2) * 2;
                    } else {
                        var iconHeight = iconSize + (PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height * plasmoid.configuration.textLines) + (4 * PlasmaCore.Units.smallSpacing);
                        if (root.isContainment && isRootView && scrollArea.viewportHeight > 0) {
                            var extraHeight = calcExtraSpacing(iconHeight, scrollArea.viewportHeight);
                            return iconHeight + extraHeight;
                        } else {
                            return iconHeight;
                        }
                    }
                }

                delegate: FolderItemDelegate {
                    width: gridView.cellWidth
                    height: gridView.cellHeight
                }

                onContentXChanged: {
                    if (hoveredItem) {
                        hoverActivateTimer.stop();
                    }

                    cancelRename();

                    dir.setDragHotSpotScrollOffset(contentX, contentY);

                    if (contentX === 0) {
                        scrollLeft = false;
                    }

                    if (contentX === contentItem.width - width) {
                        scrollRight = false;
                    }

                    // Update rubberband geometry.
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
                    if (hoveredItem) {
                        hoverActivateTimer.stop();
                    }

                    cancelRename();

                    dir.setDragHotSpotScrollOffset(contentX, contentY);

                    if (contentY === 0) {
                        scrollUp = false;
                    }

                    if (contentY === contentItem.height - height) {
                        scrollDown = false;
                    }

                    // Update rubberband geometry.
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
                        contentX = (gridView.flow === GridView.FlowLeftToRight) ? gridView.contentX : gridView.originX;
                    } else {
                        contentX = contentX;
                        smoothX.enabled = false;
                    }
                }

                onScrollRightChanged: {
                    if (scrollRight && gridView.visibleArea.widthRatio < 1.0) {
                        smoothX.enabled = true;
                        contentX = ((gridView.flow === GridView.FlowLeftToRight) ? gridView.contentX : gridView.originX)
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

                onCurrentIndexChanged: {
                    positionViewAtIndex(currentIndex, GridView.Contain);
                }

                onCachedRectangleSelectionChanged: {
                    if (cachedRectangleSelection === null) {
                        return;
                    }

                    if (cachedRectangleSelection.length) {
                        // Set current index to start of selection.
                        // cachedRectangleSelection is pre-sorted.
                        currentIndex = cachedRectangleSelection[0];
                    }

                    dir.updateSelection(cachedRectangleSelection.map(positioner.map),
                        gridView.ctrlPressed);
                }

                function makeIconSize() {
                    if (root.useListViewMode) {
                        return PlasmaCore.Units.iconSizes.small;
                    }

                    return FolderTools.iconSizeFromTheme(plasmoid.configuration.iconSize);
                }

                function updateSelection(modifier) {
                    if (modifier & Qt.ShiftModifier) {
                        positioner.setRangeSelected(anchorIndex, currentIndex);
                    } else {
                        dir.clearSelection();
                        dir.setSelected(positioner.map(currentIndex));
                        if (currentIndex === -1) {
                            previouslySelectedItemIndex = -1;
                        }
                        previouslySelectedItemIndex = currentIndex;
                    }
                }

                function cancelAutoscroll() {
                    scrollLeft = false;
                    scrollRight = false;
                    scrollUp = false;
                    scrollDown = false;
                }

                function rectangleSelect(x, y, width, height) {
                    var rows = (gridView.flow === GridView.FlowLeftToRight);
                    var axis = rows ? gridView.width : gridView.height;
                    var step = rows ? cellWidth : cellHeight;
                    var perStripe = Math.floor(axis / step);
                    var stripes = Math.ceil(gridView.count / perStripe);
                    var cWidth = gridView.cellWidth - (2 * PlasmaCore.Units.smallSpacing);
                    var cHeight = gridView.cellHeight - (2 * PlasmaCore.Units.smallSpacing);
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

                            if (gridView.effectiveLayoutDirection === Qt.RightToLeft) {
                                itemX -= (rows ? gridView.contentX : gridView.originX);
                                itemX += cWidth;
                                itemX = (rows ? gridView.width : gridView.contentItem.width) - itemX;
                            }

                            // Check if the rubberband intersects this cell first to avoid doing more
                            // expensive work.
                            if (main.rubberBand.intersects(Qt.rect(itemX + PlasmaCore.Units.smallSpacing, itemY + PlasmaCore.Units.smallSpacing,
                                cWidth, cHeight))) {
                                var item = gridView.contentItem.childAt(itemX + midWidth, itemY + midHeight);

                                // If this is a visible item, check for intersection with the actual
                                // icon or label rects for better feel.
                                if (item && item.iconArea) {
                                    var iconRect = Qt.rect(itemX + item.iconArea.x, itemY + item.iconArea.y,
                                        item.iconArea.width, item.iconArea.height);

                                    if (main.rubberBand.intersects(iconRect)) {
                                        indices.push(index);
                                        continue;
                                    }

                                    var labelRect = Qt.rect(itemX + item.labelArea.x, itemY + item.labelArea.y,
                                        item.labelArea.width, item.labelArea.height);

                                    if (main.rubberBand.intersects(labelRect)) {
                                        indices.push(index);
                                        continue;
                                    }
                                } else {
                                    // Otherwise be content with the cell intersection.
                                    indices.push(index);
                                }
                            }
                        }
                    }

                    gridView.cachedRectangleSelection = indices;
                }

                function runOrCdSelected() {
                    if (currentIndex !== -1 && dir.hasSelection()) {
                        if (root.useListViewMode && currentItem.isDir) {
                            doCd(positioner.map(currentIndex));
                        } else {
                            dir.runSelected();
                        }
                    }
                }

                Behavior on contentX { id: smoothX; enabled: false; SmoothedAnimation { velocity: 700 } }
                Behavior on contentY { id: smoothY; enabled: false; SmoothedAnimation { velocity: 700 } }

                Keys.onReturnPressed: {
                    if (event.modifiers === Qt.AltModifier) {
                        dir.openPropertiesDialog();
                    } else {
                        runOrCdSelected();
                    }
                }
                Keys.onEnterPressed: Keys.returnPressed(event)

                Keys.onMenuPressed: {
                    if (currentIndex !== -1 && dir.hasSelection() && currentItem) {
                        dir.setSelected(positioner.map(currentIndex));
                        dir.openContextMenu(currentItem.frame, event.modifiers);
                    } else {
                        // Otherwise let the containment handle it.
                        event.accepted = false;
                    }
                }

                Keys.onEscapePressed: {
                    if (!editor || !editor.targetItem) {
                        previouslySelectedItemIndex = -1;
                        dir.clearSelection();
                        event.accepted = false;
                    }
                }

                Folder.ShortCut {
                    Component.onCompleted: {
                        installAsEventFilterFor(gridView);
                    }

                    onDeleteFile: {
                        dir.deleteSelected();
                    }

                    onRenameFile: {
                        rename();
                    }

                    onCreateFolder: {
                        model.createFolder();
                    }
                }

                Keys.onPressed: {
                    event.accepted = true;

                    if (event.matches(StandardKey.Delete)) {
                        if (dir.hasSelection()) {
                            dir.action("trash").trigger();
                        }
                    } else if (event.key === Qt.Key_Control) {
                        ctrlPressed = true;
                    } else if (event.key === Qt.Key_Shift) {
                        shiftPressed = true;

                        if (currentIndex !== -1) {
                            anchorIndex = currentIndex;
                        }
                    } else if (event.key === Qt.Key_Home) {
                        currentIndex = 0;
                        updateSelection(event.modifiers);
                    } else if (event.key === Qt.Key_End) {
                        currentIndex = count - 1;
                        updateSelection(event.modifiers);
                    } else if (event.matches(StandardKey.Copy)) {
                        dir.copy();
                    } else if (event.matches(StandardKey.Paste)) {
                        dir.paste();
                    } else if (event.matches(StandardKey.Cut)) {
                        dir.cut();
                    } else if (event.matches(StandardKey.Undo)) {
                        dir.undo();
                    } else if (event.matches(StandardKey.Refresh)) {
                        dir.refresh();
                    } else if (event.matches(StandardKey.SelectAll)) {
                        positioner.setRangeSelected(0, count - 1);
                    } else {
                        event.accepted = false;
                    }
                }

                Keys.onReleased: {
                    if (event.key === Qt.Key_Control) {
                        ctrlPressed = false;
                    } else if (event.key === Qt.Key_Shift) {
                        shiftPressed = false;
                        anchorIndex = 0;
                    }
                }

                Keys.onLeftPressed: {
                    if (root.isPopup && root.useListViewMode) {
                        if (dir.resolvedUrl !== dir.resolve(plasmoid.configuration.url)) {
                            doBack();
                        }
                    } else if (positioner.enabled) {
                        var newIndex = positioner.nearestItem(currentIndex,
                            FolderTools.effectiveNavDirection(gridView.flow, gridView.effectiveLayoutDirection, Qt.LeftArrow));

                        if (newIndex !== -1) {
                            currentIndex = newIndex;
                            updateSelection(event.modifiers);
                        }
                    } else {
                        var oldIndex = currentIndex;

                        moveCurrentIndexLeft();

                        if (oldIndex === currentIndex) {
                            return;
                        }

                        updateSelection(event.modifiers);
                    }
                }

                Keys.onRightPressed: {
                    if (root.isPopup && root.useListViewMode) {
                        if (currentIndex !== -1 && dir.hasSelection() && currentItem.isDir) {
                            doCd(positioner.map(currentIndex));
                        }
                    } else if (positioner.enabled) {
                        var newIndex = positioner.nearestItem(currentIndex,
                            FolderTools.effectiveNavDirection(gridView.flow, gridView.effectiveLayoutDirection, Qt.RightArrow));

                        if (newIndex !== -1) {
                            currentIndex = newIndex;
                            updateSelection(event.modifiers);
                        }
                    } else {
                        var oldIndex = currentIndex;

                        moveCurrentIndexRight();

                        if (oldIndex === currentIndex) {
                            return;
                        }

                        updateSelection(event.modifiers);
                    }
                }

                Keys.onUpPressed: {
                    if (positioner.enabled) {
                        var newIndex = positioner.nearestItem(currentIndex,
                            FolderTools.effectiveNavDirection(gridView.flow, gridView.effectiveLayoutDirection, Qt.UpArrow));

                        if (newIndex !== -1) {
                            currentIndex = newIndex;
                            updateSelection(event.modifiers);
                        }
                    } else {
                        var oldIndex = currentIndex;

                        moveCurrentIndexUp();

                        if (oldIndex === currentIndex) {
                            return;
                        }

                        updateSelection(event.modifiers);
                    }
                }

                Keys.onDownPressed: {
                    if (positioner.enabled) {
                        var newIndex = positioner.nearestItem(currentIndex,
                            FolderTools.effectiveNavDirection(gridView.flow, gridView.effectiveLayoutDirection, Qt.DownArrow));

                        if (newIndex !== -1) {
                            currentIndex = newIndex;
                            updateSelection(event.modifiers);
                        }
                    } else {
                        var oldIndex = currentIndex;

                        moveCurrentIndexDown();

                        if (oldIndex === currentIndex) {
                            return;
                        }

                        updateSelection(event.modifiers);
                    }
                }

                Keys.onBackPressed: {
                    if (root.isPopup && dir.resolvedUrl !== dir.resolve(plasmoid.configuration.url)) {
                        doBack();
                    }
                }

                Connections {
                    target: units

                    function onIconSizesChanged() {
                        gridView.iconSize = gridView.makeIconSize();
                    }
                }

                Connections {
                    target: plasmoid.configuration

                    function onIconSizeChanged() {
                        gridView.iconSize = gridView.makeIconSize();
                    }

                    function onViewModeChanged() {
                        gridView.iconSize = gridView.makeIconSize();
                    }

                    function onUrlChanged() {
                        history = [];
                        updateHistory();
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
            parseDesktopFiles: (plasmoid.configuration.url === "desktop:/")
            previews: plasmoid.configuration.previews
            previewPlugins: plasmoid.configuration.previewPlugins
            appletInterface: plasmoid

            onListingCompleted: {
                if (!gridView.model && plasmoid.expanded) {
                    gridView.model = positioner;
                    gridView.currentIndex = isPopup ? 0 : -1;
                } else if (goingBack) {
                    goingBack = false;
                    gridView.currentIndex = Math.min(lastPosition.index, gridView.count - 1);
                    setSelected(positioner.map(gridView.currentIndex));
                    gridView.contentY = lastPosition.yPosition * gridView.contentHeight;
                }
            }

            onMove: {
                var rows = (gridView.flow === GridView.FlowLeftToRight);
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

                    if (from === -1) {
                        continue;
                    }

                    var offset = dir.dragCursorOffset(positioner.map(from));

                    if (offset.x === -1) {
                        continue;
                    }

                    itemX = dropPos.x + offset.x + (listener.dragX % cellWidth) + (cellWidth / 2);
                    itemY = dropPos.y + offset.y + (listener.dragY % cellHeight) + gridView.verticalDropHitscanOffset;

                    if (gridView.effectiveLayoutDirection === Qt.RightToLeft) {
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

                    if (from !== to) {
                        moves.push(from);
                        moves.push(to);
                    }
                }

                if (moves.length) {
                    // Update also the currentIndex, otherwise it
                    // is not set properly.
                    gridView.currentIndex = positioner.move(moves);
                    gridView.forceLayout();
                }

                previouslySelectedItemIndex = -1;
            }
        }

        Folder.Positioner {
            id: positioner

            enabled: isContainment && sortMode === -1

            folderModel: dir

            perStripe: Math.floor((gridView.flow === GridView.FlowLeftToRight)
                ? (gridView.width / gridView.cellWidth)
                : (gridView.height / gridView.cellHeight))
        }

        Folder.ItemViewAdapter {
            id: viewAdapter

            adapterView: gridView
            adapterModel: positioner
            adapterIconSize: gridView.iconSize * 2
            adapterVisibleArea: Qt.rect(gridView.contentX, gridView.contentY, gridView.contentWidth, gridView.contentHeight)

            Component.onCompleted: {
                gridView.movementStarted.connect(viewAdapter.viewScrolled);
                dir.viewAdapter = viewAdapter;
            }
        }

        Component {
            id: editorComponent

            PlasmaComponents.TextArea {
                id: editor

                visible: false

                wrapMode: root.useListViewMode ? TextEdit.NoWrap : TextEdit.Wrap

                textMargin: 0

                horizontalAlignment: root.useListViewMode ? TextEdit.AlignHLeft : TextEdit.AlignHCenter

                property Item targetItem: null

                onTargetItemChanged: {
                    if (targetItem !== null) {
                        var xy = getXY();
                        x = xy[0];
                        y = xy[1];
                        width = getWidth();
                        height = getInitHeight();
                        text = targetItem.name;
                        adjustSize();
                        editor.select(0, dir.fileExtensionBoundary(positioner.map(targetItem.index)));
                        if (isPopup) {
                            flickableItem.contentX = Math.max(flickableItem.contentWidth - contentItem.width, 0);
                        } else {
                            flickableItem.contentY = Math.max(flickableItem.contentHeight - contentItem.height, 0);
                        }
                        visible = true;
                    } else {
                        x = 0;
                        y = 0;
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

                Keys.onPressed: {
                    switch(event.key) {
                    case Qt.Key_Return:
                    case Qt.Key_Enter:
                        commit();
                        break;
                    case Qt.Key_Escape:
                        if (targetItem) {
                            targetItem = null;
                            event.accepted = true;
                        }
                        break;
                    case Qt.Key_Home:
                        if (event.modifiers & Qt.ShiftModifier) {
                            editor.select(0, cursorPosition);
                        } else {
                            editor.select(0, 0);
                        }
                        event.accepted = true;
                        break;
                    case Qt.Key_End:
                        if (event.modifiers & Qt.ShiftModifier) {
                            editor.select(cursorPosition, text.length);
                        } else {
                            editor.select(text.length, text.length);
                        }
                        event.accepted = true;
                        break;
                    default:
                        adjustSize();
                        break;
                    }
                }

                Keys.onReleased: {
                    adjustSize();
                }

                function getXY() {
                    var pos = main.mapFromItem(targetItem, targetItem.labelArea.x, targetItem.labelArea.y);
                    var _x, _y;
                    if (root.useListViewMode) {
                       _x = targetItem.labelArea.x - __style.padding.left;
                       _y = pos.y - __style.padding.top;
                    } else {
                        _x = targetItem.x + Math.abs(Math.min(gridView.contentX, gridView.originX));
                        _x += __style.padding.left;
                        _x += scrollArea.viewport.x;
                        if (verticalScrollBarPolicy === Qt.ScrollBarAlwaysOn
                            && gridView.effectiveLayoutDirection === Qt.RightToLeft) {
                            _x -= __verticalScrollBar.parent.verticalScrollbarOffset;
                        }
                        _y = pos.y + PlasmaCore.Units.smallSpacing - __style.padding.top;
                    }
                    return [ _x, _y ];
                }

                function getWidth(addWidthVerticalScroller) {
                     return(targetItem.label.parent.width - PlasmaCore.Units.smallSpacing +
                            (root.useListViewMode ? -(__style.padding.left + __style.padding.right + PlasmaCore.Units.smallSpacing) : 0) +
                            (addWidthVerticalScroller ? __verticalScrollBar.parent.verticalScrollbarOffset : 0));
                }

                function getHeight(addWidthHoriozontalScroller, init) {
                    var _height;
                    if (isPopup || init) {
                        _height = targetItem.labelArea.height + __style.padding.top + __style.padding.bottom;
                    } else {
                        var realHeight = contentHeight + __style.padding.top + __style.padding.bottom;
                        var maxHeight = PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height * (plasmoid.configuration.textLines + 1) + __style.padding.top + __style.padding.bottom;
                        _height = Math.min(realHeight, maxHeight);
                    }
                    return _height + (addWidthHoriozontalScroller ? __horizontalScrollBar.parent.horizontalScrollbarOffset : 0);
                }

                function getInitHeight() {
                    return getHeight(false, true);
                }

                function adjustSize() {
                    if (isPopup) {
                        if(contentWidth + __style.padding.left + __style.padding.right > width) {
                            visible = true;
                            horizontalScrollBarPolicy = Qt.ScrollBarAlwaysOn;
                            height = getHeight(true);
                        } else {
                            horizontalScrollBarPolicy = Qt.ScrollBarAlwaysOff;
                            height = getHeight();
                        }
                    } else {
                        height = getHeight();
                        if(contentHeight + __style.padding.top + __style.padding.bottom > height) {
                            visible = true;
                            verticalScrollBarPolicy = Qt.ScrollBarAlwaysOn;
                            width = getWidth(true);
                        } else {
                            verticalScrollBarPolicy = Qt.ScrollBarAlwaysOff;
                            width = getWidth();
                        }
                    }

                    var xy = getXY();
                    x = xy[0];
                    y = xy[1];
                }

                function commit() {
                    if (targetItem) {
                        dir.rename(positioner.map(targetItem.index), text);
                        targetItem = null;
                    }
                }
            }
        }

        Component.onCompleted: {
            dir.requestRename.connect(rename);
        }
    }

    Component.onCompleted: {
        if (backButton === null && root.useListViewMode) {
            backButton = makeBackButton();
        }
    }
}
