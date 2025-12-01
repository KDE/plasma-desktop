/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQml

import org.kde.plasma.plasmoid
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.kquickcontrolsaddons

import org.kde.private.desktopcontainment.folder as Folder
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
    property alias positionerApplet: positioner.applet
    property alias errorString: dir.errorString
    property alias dragging: dir.dragging
    property alias dragInProgressAnywhere: dir.dragInProgressAnywhere
    property alias locked: dir.locked
    property alias sortMode: dir.sortMode
    property alias filterMode: dir.filterMode
    property alias filterPattern: dir.filterPattern
    property alias filterMimeTypes: dir.filterMimeTypes
    property alias showHiddenFiles: dir.showHiddenFiles
    property alias creatingNewItems: dir.creatingNewItems
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
            let renameAction = folderView.model.action("rename");
            if (renameAction && !renameAction.enabled) {
                return;
            }

            if (!main.editor) {
                editor = editorComponent.createObject(listener);
            }

            main.editor.targetItem = gridView.currentItem;
        }
    }

    function cancelRename() {
        if (main.editor) {
            main.editor.targetItem = null;
        }
    }

    function linkHere(sourceUrl) {
        dir.linkHere(sourceUrl);
    }

    function handleDragMove(x, y) {
        let child = childAt(x, y);

        if (child !== null && child === main.backButton) {
            hoveredItem = null;
            main.backButton.handleDragMove();
        } else {
            if (main.backButton && main.backButton.containsDrag) {
                main.backButton.endDragMove();
            }

            let pos = mapToItem(gridView.contentItem, x, y);
            let item = gridView.safeItemAt(pos.x, pos.y);

            if (item && item.isDir) {
                hoveredItem = item;
            } else {
                hoveredItem = null;
            }
        }
    }

    function endDragMove() {
        if (main.backButton && main.backButton.active) {
            main.backButton.endDragMove();
        } else if (hoveredItem && !hoveredItem.popupDialog) {
            hoveredItem = null;
        }
    }

    function dropItemAt(pos) {
        let item = gridView.safeItemAt(pos.x, pos.y);

        if (item && !item.blank) {
            gridView.currentIndex = item.index
            return positioner.map(item.index);
        }

        return -1;
    }

    function drop(target, event, pos) {
        let dropPos = mapToItem(gridView.contentItem, pos.x, pos.y);
        let dropIndex = gridView.safeIndexAt(dropPos.x, dropPos.y);
        let dragPos = mapToItem(gridView.contentItem, listener.dragX, listener.dragY);
        let dragIndex = gridView.safeIndexAt(dragPos.x, dragPos.y);

        if (listener.dragX === -1 || dragIndex !== dropIndex) {
            dir.drop(target, event, dropItemAt(dropPos), root.isContainment && !Plasmoid.immutable);
        }
    }

    function generateDragImage() {
        for (let i = 0; i < gridView.count; i++) {
            let item = gridView.itemAtIndex(i);
            if (item) {
                item.updateDragImage();
            }
        }
    }

    Connections {
        target: dir
        function onPopupMenuAboutToShow(dropJob, mimeData, x, y) {
            if (root.isContainment && !Plasmoid.immutable) {
                root.processMimeData(mimeData, x, y, dropJob);
            }
        }

        // Create drag images before dragging
        // Due to async operations we can't call this before dragging starts,
        // but we have to call it after a selection is done
        function onSelectionDone() {
            main.generateDragImage();
        }
    }

    Connections {
        target: root
        function onExpandedChanged() {
            if (root.expanded && dir.status === Folder.FolderModel.Ready && !gridView.model) {
                gridView.model = positioner;
            }
        }
    }

    Binding {
        target: Plasmoid
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
            if (main.backButton === null && root.useListViewMode) {
                main.backButton = main.makeBackButton();
            } else if (main.backButton !== null) {
                main.backButton.destroy();
            }
        }
    }

    Folder.EventGenerator {
        id: eventGenerator
    }

    onEnabledChanged: {
        if (!main.enabled) {
            listener.clearPressState();
        }
    }

    MouseEventListener {
        id: listener

        enabled: !Plasmoid.containment.corona.editMode

        anchors {
            topMargin: main.backButton !== null ? main.backButton.height : undefined
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
        property bool renameByLabelClickInitiated: false

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

        onPressed: mouse => {

            // Ignore press events outside the viewport (i.e. on scrollbars).
            if (!scrollArea.viewport.contains(Qt.point(mouse.x, mouse.y))) {
                return;
            }

            // Ignore clicks if main.editor is enabled and we click on that
            // BUG:494558
            if (main.editor && childAt(mouse.x, mouse.y) === main.editor) {
                return;
            }

            scrollArea.focus = true;

            if (mouse.buttons & Qt.BackButton) {
                if (root.isPopup && dir.resolvedUrl !== dir.resolve(Plasmoid.configuration.url)) {
                    main.doBack();
                    mouse.accepted = true;
                }

                return;
            }

            if (main.editor && childAt(mouse.x, mouse.y) !== main.editor) {
                main.editor.commit();
            }

            const mappedPos = mapToItem(gridView.contentItem, mouse.x, mouse.y)
            const index = gridView.safeIndexAt(mappedPos.x, mappedPos.y);
            const indexItem = gridView.itemAtIndex(index);

            if (indexItem && indexItem.iconArea) { // update position in case of touch or untriggered hover
                gridView.currentIndex = index;
                hoveredItem = indexItem;
            } else {
                gridView.currentIndex = -1
                hoveredItem = null;
            }

            if (mouse.source === Qt.MouseEventSynthesizedByQt) {
                if (gridView.hoveredItem && gridView.hoveredItem.toolTip.active) {
                    gridView.hoveredItem.toolTip.hideToolTip();
                }
            }

            pressX = mouse.x;
            pressY = mouse.y;

            if (!hoveredItem || hoveredItem.blank) {
                if (!gridView.ctrlPressed) {
                    gridView.currentIndex = -1;
                    main.previouslySelectedItemIndex = -1;
                    dir.clearSelection();
                }

                if (mouse.buttons & Qt.RightButton) {
                    clearPressState();

                    // If it's the desktop, fall through to the desktop context menu plugin
                    // Disallow opening contextmenu if we're already creating new items
                    if (!dir.usedByContainment && !dir.creatingNewItems) {
                        dir.openContextMenu(main, mouse.modifiers);
                        mouse.accepted = true;
                    }
                }
            } else {
                pressedItem = hoveredItem;

                let pos = mapToItem(hoveredItem.actionsOverlay, mouse.x, mouse.y);

                if (!(pos.x <= hoveredItem.actionsOverlay.width && pos.y <= hoveredItem.actionsOverlay.height)) {
                    if (gridView.shiftPressed && gridView.currentIndex !== -1) {
                        positioner.setRangeSelected(gridView.anchorIndex, hoveredItem.index);
                    } else {
                        // Deselecting everything else when one item is clicked is handled in onReleased in order to distinguish between drag and click
                        if (!gridView.ctrlPressed && !dir.isSelected(positioner.map(hoveredItem.index))) {
                            main.previouslySelectedItemIndex = -1;
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

        onReleased: mouse => {
            // if we click on an item, cancel the current selection and select just the clicked icon
            // the cachedRectangleSelection guards this release being associated with an existing drag
            if (!gridView.cachedRectangleSelection && hoveredItem && !hoveredItem.blank && mouse.button !== Qt.RightButton) {
                const pos = mapToItem(hoveredItem.actionsOverlay, mouse.x, mouse.y);
                if (!(pos.x <= hoveredItem.actionsOverlay.width && pos.y <= hoveredItem.actionsOverlay.height)
                    && (!(gridView.shiftPressed && gridView.currentIndex !== -1) && !gridView.ctrlPressed)) {
                        dir.clearSelection();
                        dir.setSelected(positioner.map(hoveredItem.index));
                }
            }
            pressCanceled();
        }

        onPressAndHold: mouse => {
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

        onClicked: mouse => {
            clearPressState();

            if (mouse.button === Qt.RightButton ||
                (main.editor && childAt(mouse.x, mouse.y) === main.editor)) {
                return;
            }

            if (!hoveredItem || hoveredItem.blank || gridView.currentIndex === -1 || gridView.ctrlPressed || gridView.shiftPressed) {
                // Bug 357367: Replay mouse event, so containment actions assigned to left mouse button work.
                eventGenerator.sendMouseEvent(root, Folder.EventGenerator.MouseButtonPress, mouse.x, mouse.y, mouse.button, mouse.buttons, mouse.modifiers);
                return;
            }

            let pos = mapToItem(hoveredItem, mouse.x, mouse.y);

            // Moving from an item to its preview popup dialog doesn't unset hoveredItem
            // even though the cursor has left it, so we need to check whether the click
            // actually occurred inside the item we expect it in before going ahead. If it
            // didn't, clean up (e.g. dismissing the dialog as a side-effect of unsetting
            // hoveredItem) and abort.
            if (pos.x < 0 || pos.x > hoveredItem.width || pos.y < 0 || pos.y > hoveredItem.height) {
                hoveredItem = null;
                main.previouslySelectedItemIndex = -1;
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

                // Clicked on the label of an already-selected item: schedule it for renaming when doubleClickTimer expires
                renameByLabelClickInitiated = (pos.x > hoveredItem.labelArea.x
                    && pos.x <= hoveredItem.labelArea.x + hoveredItem.labelArea.width
                    && pos.y > hoveredItem.labelArea.y
                    && pos.y <= hoveredItem.labelArea.y + hoveredItem.labelArea.height
                    && main.previouslySelectedItemIndex === gridView.currentIndex
                    && gridView.currentIndex !== -1
                    && !Qt.styleHints.singleClickActivation
                    && Plasmoid.configuration.renameInline
                )

                // Single-click mode and single-clicked on the item or
                // double-click mode and double-clicked on the item: activate it
                if (Qt.styleHints.singleClickActivation || doubleClickInProgress || mouse.source === Qt.MouseEventSynthesizedByQt) {
                    doubleClickInProgress = false
                    if (mouse.modifiers & Qt.AltModifier) {
                        dir.openPropertiesDialog();
                    } else if (root.useListViewMode && mouse.button === Qt.LeftButton && hoveredItem.isDir) {
                        main.doCd(positioner.map(gridView.currentIndex));
                    } else {
                        dir.run(positioner.map(gridView.currentIndex));
                    }
                    main.previouslySelectedItemIndex = gridView.currentIndex;
                    hoveredItem = null;
                } else {
                    // None of the above: select it
                    doubleClickInProgress = true;
                    doubleClickTimer.interval = Qt.styleHints.mouseDoubleClickInterval;
                    doubleClickTimer.start();
                    main.previouslySelectedItemIndex = gridView.currentIndex;
                }
            }
        }

        onPositionChanged: mouse => {
            gridView.ctrlPressed = (mouse.modifiers & Qt.ControlModifier);
            gridView.shiftPressed = (mouse.modifiers & Qt.ShiftModifier);

            const mappedPos = mapToItem(gridView.contentItem, mouse.x, mouse.y)
            const item = gridView.safeItemAt(mappedPos.x, mappedPos.y);
            const leftEdge = Math.min(gridView.contentX, gridView.originX);

            if (!item || item.blank) {
                if (gridView.hoveredItem && !root.containsDrag && (!main.dialog || !main.dialog.containsDrag) && !gridView.hoveredItem.popupDialog) {
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
                let rB = main.rubberBand;
                let cPos = mapToItem(gridView.contentItem, mouse.x, mouse.y);

                if (cPos.x < cPress.x) {
                    rB.x = Math.max(leftEdge, cPos.x);
                    rB.width = Math.abs(rB.x - cPress.x);
                } else {
                    rB.x = cPress.x;
                    let ceil = Math.max(gridView.width, gridView.contentItem.width) + leftEdge;
                    rB.width = Math.min(ceil - rB.x, Math.abs(rB.x - cPos.x));
                }

                if (cPos.y < cPress.y) {
                    rB.y = Math.max(0, cPos.y);
                    rB.height = Math.abs(rB.y - cPress.y);
                } else {
                    rB.y = cPress.y;
                    let ceil = Math.max(gridView.height, gridView.contentItem.height);
                    rB.height = Math.min(ceil - rB.y, Math.abs(rB.y - cPos.y));
                }

                // Ensure rubberband is at least 1px in size or else it will become
                // invisible and not match any items.
                rB.width = Math.max(1, rB.width);
                rB.height = Math.max(1, rB.height);

                Qt.callLater(gridView.rectangleSelect, rB.x, rB.y, rB.width, rB.height, main.rubberBand);

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
                    if (root.useListViewMode || (main.editor && main.editor.targetItem) || (verticalScrollBar.active || horizontalScrollBar.active)) {
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

            Rectangle {
                id: rubberBand

                width: 0
                height: 0
                z: 99999

                radius: Kirigami.Units.cornerRadius
                border.color: Kirigami.Theme.highlightColor
                color: Qt.alpha(border.color, 0.3)

                function intersects(rect) {
                    return x + width >= rect.x && y + height >= rect.y && rect.x + rect.width >= x && rect.y + rect.height >= y;
                }

                function close() {
                    opacityAnimation.restart();
                }

                OpacityAnimator {
                    id: opacityAnimation
                    target: rubberBand
                    to: 0
                    from: 1
                    duration: Kirigami.Units.shortDuration

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
                        // We need to explicitly generate an image here
                        // to make sure we have one before we start dragging
                        main.generateDragImage();
                        rubberBand.destroy();
                    }
                }
            }
        }

        onContainsMouseChanged: {
            if (!containsMouse && !main.rubberBand) {
                clearPressState();
                if (!(hoveredItem?.popupDialog?.visible ?? false)) {
                    hoveredItem = null;
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
                if (listener.renameByLabelClickInitiated && listener.doubleClickInProgress) {
                    main.rename()
                }
                listener.renameByLabelClickInitiated = false
                listener.doubleClickInProgress = false;
            }
        }

        Timer {
            id: hoverActivateTimer

            interval: root.hoverActivateDelay

            onTriggered: {
                if (!main.hoveredItem) {
                    return;
                }

                if (root.useListViewMode) {
                    main.doCd(index);
                } else if (Plasmoid.configuration.popups) {
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

                // itemAt returns the item that's in the cell at
                // coordinates x, y, even if it's smaller than it.
                // safeItemAt checks if x, y is actually within the
                // element within the cell.
                function safeItemAt(x, y) {
                    let item = itemAt(x, y)
                    if (!item) return;
                    let coord = mapFromItem(gridView.contentItem, x, y)
                    coord = item.mapFromItem(gridView, coord.x, coord.y)
                    if (!item.contains(coord)) {
                        return
                    }
                    return item;
                }

                function safeIndexAt(x, y) {
                    if (safeItemAt(x, y)) {
                        return indexAt(x, y)
                    }
                    return -1
                }

                PlasmaComponents.ScrollBar.vertical: PlasmaComponents.ScrollBar {
                    id: verticalScrollBar
                }
                PlasmaComponents.ScrollBar.horizontal: PlasmaComponents.ScrollBar {
                    id: horizontalScrollBar
                }

                function calcExtraSpacing(cellSize, containerSize) {
                    const availableColumns = Math.floor(containerSize / cellSize);
                    let extraSpacing = 0;
                    if (availableColumns > 0) {
                        let allColumnSize = availableColumns * cellSize;
                        let extraSpace = Math.max(containerSize - allColumnSize, 0);
                        extraSpacing = extraSpace / availableColumns;
                    }
                    return Math.floor(extraSpacing);
                }

                cellWidth: {
                    if (root.useListViewMode) {
                        return gridView.width - (verticalScrollBar.visible ? verticalScrollBar.width : 0);
                    } else {
                        const iconWidth = iconSize + (2 * Kirigami.Units.gridUnit) + (2 * Kirigami.Units.smallSpacing);
                        if (root.isContainment && isRootView && scrollArea.viewportWidth > 0) {
                            const minIconWidth = Math.max(iconWidth, Kirigami.Units.iconSizes.small * ((Plasmoid.configuration.labelWidth * 2) + 4));
                            const extraWidth = calcExtraSpacing(minIconWidth, scrollArea.viewportWidth);
                            return minIconWidth + extraWidth;
                        } else {
                            return iconWidth;
                        }
                    }
                }

                cellHeight: {
                    if (root.useListViewMode) {
                        return Math.ceil((Math.max(Kirigami.Units.iconSizes.sizeForLabels, iconSize)
                            + Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
                            listItemSvg.margins.top + listItemSvg.margins.bottom)) / 2) * 2;
                    } else {
                        // the smallSpacings are for padding
                        const iconHeight = iconSize + (Kirigami.Units.gridUnit * Plasmoid.configuration.textLines) + (Kirigami.Units.smallSpacing * 3);
                        if (root.isContainment && isRootView && scrollArea.viewportHeight > 0) {
                            let extraHeight = calcExtraSpacing(iconHeight, scrollArea.viewportHeight);
                            return iconHeight + extraHeight;
                        } else {
                            return iconHeight;
                        }
                    }
                }

                delegate: FolderItemDelegate {
                    width: gridView.cellWidth
                    height: contentHeight || gridView.cellHeight
                    isOnRootView: main.isRootView
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
                        const rB = main.rubberBand;

                        if (scrollLeft) {
                            rB.x = Math.min(gridView.contentX, gridView.originX);
                            rB.width = listener.cPress.x;
                        }

                        if (scrollRight) {
                            const lastCol = gridView.contentX + gridView.width;
                            rB.width = lastCol - rB.x;
                        }

                        Qt.callLater(gridView.rectangleSelect, rB.x, rB.y, rB.width, rB.height, main.rubberBand);
                    }
                }

                onContentYChanged: {
                    if (hoveredItem) {
                        hoverActivateTimer.stop();
                    }

                    main.cancelRename();

                    dir.setDragHotSpotScrollOffset(contentX, contentY);

                    if (contentY === 0) {
                        scrollUp = false;
                    }

                    if (contentY === contentItem.height - height) {
                        scrollDown = false;
                    }

                    // Update rubberband geometry.
                    if (main.rubberBand) {
                        const rB = main.rubberBand;

                        if (scrollUp) {
                            rB.y = 0;
                            rB.height = listener.cPress.y;
                        }

                        if (scrollDown) {
                            const lastRow = gridView.contentY + gridView.height;
                            rB.height = lastRow - rB.y;
                        }

                        Qt.callLater(gridView.rectangleSelect, rB.x, rB.y, rB.width, rB.height, main.rubberBand);
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

                    dir.updateSelection(cachedRectangleSelection.map(row => positioner.map(row)),
                        gridView.ctrlPressed);
                }

                function makeIconSize() {
                    if (root.useListViewMode) {
                        return Kirigami.Units.iconSizes.small;
                    }

                    return FolderTools.iconSizeFromTheme(Plasmoid.configuration.iconSize);
                }

                function updateSelection(modifier) {
                    if (modifier & Qt.ShiftModifier) {
                        positioner.setRangeSelected(anchorIndex, currentIndex);
                    } else {
                        dir.clearSelection();
                        dir.setSelected(positioner.map(currentIndex));
                        if (currentIndex === -1) {
                            main.previouslySelectedItemIndex = -1;
                        }
                        main.previouslySelectedItemIndex = currentIndex;
                    }
                }

                function cancelAutoscroll() {
                    scrollLeft = false;
                    scrollRight = false;
                    scrollUp = false;
                    scrollDown = false;
                }

                function rectangleSelect(x, y, width, height, rubberBand) {
                    const rows = (gridView.flow === GridView.FlowLeftToRight);
                    const axis = rows ? gridView.width : gridView.height;
                    const step = rows ? cellWidth : cellHeight;
                    const stripes = Math.ceil(gridView.count / positioner.perStripe);
                    const cWidth = gridView.cellWidth - (2 * Kirigami.Units.smallSpacing);
                    const cHeight = gridView.cellHeight - (2 * Kirigami.Units.smallSpacing);
                    const midWidth = gridView.cellWidth / 2;
                    const midHeight = gridView.cellHeight / 2;
                    let indices = [];

                    for (let s = 0; s < stripes; s++) {
                        for (let i = 0; i < positioner.perStripe; i++) {
                            let index = (s * positioner.perStripe) + i;

                            if (index >= gridView.count) {
                                break;
                            }

                            if (positioner.isBlank(index)) {
                                continue;
                            }

                            let itemX = ((rows ? i : s) * gridView.cellWidth);
                            let itemY = ((rows ? s : i) * gridView.cellHeight);

                            if (gridView.effectiveLayoutDirection === Qt.RightToLeft) {
                                itemX -= (rows ? gridView.contentX : gridView.originX);
                                itemX += cWidth;
                                itemX = (rows ? gridView.width : gridView.contentItem.width) - itemX;
                            }

                            const item = gridView.contentItem.childAt(itemX + midWidth, itemY + midHeight);
                            if (item && rubberBand.intersects(Qt.rect(item.x, item.y, item.width, item.height))) {
                                indices.push(index)
                            }

                        }
                    }

                    gridView.cachedRectangleSelection = indices;
                }

                function runOrCdSelected() {
                    if (currentIndex !== -1 && dir.hasSelection()) {
                        if (root.useListViewMode && currentItem.isDir) {
                            main.doCd(positioner.map(currentIndex));
                        } else {
                            dir.runSelected();
                        }
                    }
                }

                Behavior on contentX { id: smoothX; enabled: false; SmoothedAnimation { velocity: 700 } }
                Behavior on contentY { id: smoothY; enabled: false; SmoothedAnimation { velocity: 700 } }

                Keys.onReturnPressed: event => {
                    if (event.modifiers === Qt.AltModifier) {
                        dir.openPropertiesDialog();
                    } else {
                        runOrCdSelected();
                    }
                }

                Keys.onEnterPressed: event => Keys.returnPressed(event)

                Keys.onMenuPressed: event => {
                    if (currentIndex !== -1 && dir.hasSelection() && currentItem) {
                        dir.setSelected(positioner.map(currentIndex));
                        dir.openContextMenu(currentItem.frame, event.modifiers);
                    } else {
                        // Otherwise let the containment handle it.
                        event.accepted = false;
                    }
                }

                Keys.onEscapePressed: event => {
                    if (!main.editor || !main.editor.targetItem) {
                        main.previouslySelectedItemIndex = -1;
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
                        main.rename();
                    }

                    onMoveToTrash: {
                        const action = dir.action("trash");
                        if (action && action.enabled) {
                            action.trigger();
                        }
                    }

                    onCreateFolder: {
                        model.createFolder();
                    }
                }

                Keys.onPressed: event => {
                    event.accepted = true;

                    if (event.key === Qt.Key_Control) {
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

                Keys.onReleased: event => {
                    if (event.key === Qt.Key_Control) {
                        ctrlPressed = false;
                    } else if (event.key === Qt.Key_Shift) {
                        shiftPressed = false;
                        anchorIndex = 0;
                    }
                }

                Keys.onLeftPressed: event => {
                    if (root.isPopup && root.useListViewMode) {
                        if (dir.resolvedUrl !== dir.resolve(Plasmoid.configuration.url)) {
                            main.doBack();
                        }
                    } else if (positioner.enabled) {
                        const newIndex = positioner.nearestItem(currentIndex,
                            FolderTools.effectiveNavDirection(gridView.flow, gridView.effectiveLayoutDirection, Qt.LeftArrow));

                        if (newIndex !== -1) {
                            currentIndex = newIndex;
                            updateSelection(event.modifiers);
                        }
                    } else {
                        const oldIndex = currentIndex;

                        moveCurrentIndexLeft();

                        if (oldIndex === currentIndex) {
                            return;
                        }

                        updateSelection(event.modifiers);
                    }
                }

                Keys.onRightPressed: event => {
                    if (root.isPopup && root.useListViewMode) {
                        if (currentIndex !== -1 && dir.hasSelection() && currentItem.isDir) {
                            main.doCd(positioner.map(currentIndex));
                        }
                    } else if (positioner.enabled) {
                        const newIndex = positioner.nearestItem(currentIndex,
                            FolderTools.effectiveNavDirection(gridView.flow, gridView.effectiveLayoutDirection, Qt.RightArrow));

                        if (newIndex !== -1) {
                            currentIndex = newIndex;
                            updateSelection(event.modifiers);
                        }
                    } else {
                        const oldIndex = currentIndex;

                        moveCurrentIndexRight();

                        if (oldIndex === currentIndex) {
                            return;
                        }

                        updateSelection(event.modifiers);
                    }
                }

                Keys.onUpPressed: event => {
                    if (positioner.enabled) {
                        const newIndex = positioner.nearestItem(currentIndex,
                            FolderTools.effectiveNavDirection(gridView.flow, gridView.effectiveLayoutDirection, Qt.UpArrow));

                        if (newIndex !== -1) {
                            currentIndex = newIndex;
                            updateSelection(event.modifiers);
                        }
                    } else {
                        const oldIndex = currentIndex;

                        moveCurrentIndexUp();

                        if (oldIndex === currentIndex) {
                            return;
                        }

                        updateSelection(event.modifiers);
                    }
                }

                Keys.onDownPressed: event => {
                    if (positioner.enabled) {
                        const newIndex = positioner.nearestItem(currentIndex,
                            FolderTools.effectiveNavDirection(gridView.flow, gridView.effectiveLayoutDirection, Qt.DownArrow));

                        if (newIndex !== -1) {
                            currentIndex = newIndex;
                            updateSelection(event.modifiers);
                        }
                    } else {
                        const oldIndex = currentIndex;

                        moveCurrentIndexDown();

                        if (oldIndex === currentIndex) {
                            return;
                        }

                        updateSelection(event.modifiers);
                    }
                }

                Keys.onBackPressed: event => {
                    if (root.isPopup && dir.resolvedUrl !== dir.resolve(Plasmoid.configuration.url)) {
                        main.doBack();
                    }
                }

                Connections {
                    target: Plasmoid.configuration

                    function onIconSizeChanged() {
                        gridView.iconSize = gridView.makeIconSize();
                    }

                    function onViewModeChanged() {
                        gridView.iconSize = gridView.makeIconSize();
                    }

                    function onUrlChanged() {
                        main.history = [];
                        main.updateHistory();
                    }
                }
            }

            Kirigami.InlineMessage {
                width: parent.width / 2.0
                anchors.horizontalCenter: parent.horizontalCenter
                type: Kirigami.MessageType.Warning
                text: i18nc("@info",
                    "There are a lot of files and folders on the desktop. This can cause bugs and performance issues. Please consider moving some of them elsewhere.")
                // Note: the trigger amount is intentionally lower than the screen mapping cap. We want to warn ahead of hitting our caps.
                visible: main.isRootView && gridView.count > 2048
            }
        }

        Folder.WheelInterceptor {
            anchors.fill: parent

            enabled: root.isContainment && !gridView.overflowing
            destination: root
        }

        Folder.FolderModel {
            id: dir

            usedByContainment: root.isContainment && main.isRootView
            sortDesc: Plasmoid.configuration.sortDesc
            sortDirsFirst: Plasmoid.configuration.sortDirsFirst
            parseDesktopFiles: (Plasmoid.configuration.url === "desktop:/")
            previews: Plasmoid.configuration.previews
            previewPlugins: Plasmoid.configuration.previewPlugins
            applet: Plasmoid

            onListingCompleted: {
                if (!gridView.model && root.expanded) {
                    gridView.model = positioner;
                    gridView.currentIndex = isPopup ? 0 : -1;
                } else if (main.goingBack) {
                    main.goingBack = false;
                    gridView.currentIndex = Math.min(main.lastPosition.index, gridView.count - 1);
                    setSelected(positioner.map(gridView.currentIndex));
                    gridView.contentY = main.lastPosition.yPosition * gridView.contentHeight;
                }
            }

            onMove: (x, y, urls) => {
                if (!positioner.enabled) {
                    return;
                }
                const rows = (gridView.flow === GridView.FlowLeftToRight);
                const dropPos = gridView.mapToItem(gridView.contentItem, x, y);
                const leftEdge = Math.min(gridView.contentX, gridView.originX);

                let moves = []
                let itemX = -1;
                let itemY = -1;
                let col = -1;
                let row = -1;
                let from = -1;
                let to = -1;

                for (let i = 0; i < urls.length; i++) {
                    from = positioner.indexForUrl(urls[i]);
                    to = -1;

                    if (from === -1) {
                        continue;
                    }

                    let offset = dir.dragCursorOffset(positioner.map(from));

                    if (offset.x === -1) {
                        continue;
                    }

                    // The +(gridView.cellWidth / 2) is kept here to make it easier to drag items between cells.
                    itemX = dropPos.x + offset.x + (listener.dragX % gridView.cellWidth) + (gridView.cellWidth / 2);
                    itemY = dropPos.y + offset.y + (listener.dragY % gridView.cellHeight) + gridView.verticalDropHitscanOffset;


                    if (gridView.effectiveLayoutDirection === Qt.RightToLeft) {
                        itemX -= (rows ? gridView.contentX : gridView.originX);
                        itemX = (rows ? gridView.width : gridView.contentItem.width) - itemX;
                    }

                    col = rows ? Math.floor(itemX / gridView.cellWidth) : Math.floor(itemY / gridView.cellHeight);
                    row = rows ? Math.floor(itemY / gridView.cellHeight) : Math.floor(itemX / gridView.cellWidth);


                    if (col <= positioner.perStripe) {
                        // We have somehow moved the item outside of the available
                        // areas (usually during file creation), so make sure
                        // the col is within perStripe
                        if (col === positioner.perStripe) {
                            col -= 1;
                        }

                        to = (row * positioner.perStripe) + col;

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

                main.previouslySelectedItemIndex = -1;
            }
        }

        Folder.Positioner {
            id: positioner

            enabled: main.isRootView && main.sortMode === -1

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

            RenameEditor {
                id: editor

                visible: false

                onCommit: {
                    if (targetItem) {
                        dir.rename(positioner.map(targetItem.index), text);
                        targetItem = null;
                        gridView.forceActiveFocus();
                    }
                }

                onVisibleChanged: {
                    if (root.visible) {
                        focus = true;
                    } else {
                        scrollArea.focus = true;
                    }
                }
            }
        }

        Component.onCompleted: {
            dir.requestRename.connect(main.rename);
        }
    }

    Component.onCompleted: {
        if (main.backButton === null && root.useListViewMode) {
            main.backButton = makeBackButton();
        }
    }
}
