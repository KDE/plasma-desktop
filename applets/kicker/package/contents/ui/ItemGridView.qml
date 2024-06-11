/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras

FocusScope {
    id: itemGrid

    signal keyNavLeft
    signal keyNavRight
    signal keyNavUp
    signal keyNavDown

    signal itemActivated(int index, string actionId, string argument)

    property bool dragEnabled: true
    property bool dropEnabled: false
    property bool showLabels: true

    property alias currentIndex: gridView.currentIndex
    property alias currentItem: gridView.currentItem
    property alias contentItem: gridView.contentItem
    property alias count: gridView.count
    property alias model: gridView.model

    property alias cellWidth: gridView.cellWidth
    property alias cellHeight: gridView.cellHeight
    property alias iconSize: gridView.iconSize

    property var horizontalScrollBarPolicy: PlasmaComponents.ScrollBar.AlwaysOff
    property var verticalScrollBarPolicy: PlasmaComponents.ScrollBar.AsNeeded

    onDropEnabledChanged: {
        if (!dropEnabled && "dropPlaceHolderIndex" in model) {
            model.dropPlaceHolderIndex = -1;
        }
    }

    onFocusChanged: {
        if (!focus && !root.keyEventProxy.activeFocus) {
            currentIndex = -1;
        }
    }

    function currentRow() {
        if (currentIndex === -1) {
            return -1;
        }

        return Math.floor(currentIndex / Math.floor(width / itemGrid.cellWidth));
    }

    function currentCol() {
        if (currentIndex === -1) {
            return -1;
        }

        return currentIndex - (currentRow() * Math.floor(width / itemGrid.cellWidth));
    }

    function lastRow() {
        var columns = Math.floor(width / itemGrid.cellWidth);
        return Math.ceil(count / columns) - 1;
    }

    function tryActivate(row, col) {
        if (count) {
            var columns = Math.floor(width / itemGrid.cellWidth);
            var rows = Math.ceil(count / columns);
            row = Math.min(row, rows - 1);
            col = Math.min(col, columns - 1);
            currentIndex = Math.min(row ? ((Math.max(1, row) * columns) + col)
                : col,
                count - 1);

            focus = true;
        }
    }

    function forceLayout() {
        gridView.forceLayout();
    }

    ActionMenu {
        id: actionMenu

        onActionClicked: (actionId, actionArgument) => {
            visualParent.actionTriggered(actionId, actionArgument);
        }
    }

    DropArea {
        id: dropArea

        anchors.fill: parent

        onPositionChanged: event => {
            if (!itemGrid.dropEnabled || gridView.animating || !kicker.dragSource) {
                return;
            }

            var x = Math.max(0, event.x - (width % itemGrid.cellWidth));
            var cPos = mapToItem(gridView.contentItem, x, event.y);
            var item = gridView.itemAt(cPos.x, cPos.y);

            if (item) {
                if (kicker.dragSource.parent === gridView.contentItem) {
                    if (item !== kicker.dragSource) {
                        item.GridView.view.model.moveRow(dragSource.itemIndex, item.itemIndex);
                    }
                } else if (kicker.dragSource.GridView.view.model.favoritesModel === itemGrid.model
                    && !itemGrid.model.isFavorite(kicker.dragSource.favoriteId)) {
                    var hasPlaceholder = (itemGrid.model.dropPlaceholderIndex !== -1);

                    itemGrid.model.dropPlaceholderIndex = item.itemIndex;

                    if (!hasPlaceholder) {
                        gridView.currentIndex = (item.itemIndex - 1);
                    }
                }
            } else if (kicker.dragSource.parent !== gridView.contentItem
                && kicker.dragSource.GridView.view.model.favoritesModel === itemGrid.model
                && !itemGrid.model.isFavorite(kicker.dragSource.favoriteId)) {
                    var hasPlaceholder = (itemGrid.model.dropPlaceholderIndex !== -1);

                    itemGrid.model.dropPlaceholderIndex = hasPlaceholder ? itemGrid.model.count - 1 : itemGrid.model.count;

                    if (!hasPlaceholder) {
                        gridView.currentIndex = (itemGrid.model.count - 1);
                    }
            } else {
                itemGrid.model.dropPlaceholderIndex = -1;
                gridView.currentIndex = -1;
            }
        }

        onExited: {
            if ("dropPlaceholderIndex" in itemGrid.model) {
                itemGrid.model.dropPlaceholderIndex = -1;
                gridView.currentIndex = -1;
            }
        }

        onDropped: drop => {
            if (kicker.dragSource && kicker.dragSource.parent !== gridView.contentItem && kicker.dragSource.GridView.view.model.favoritesModel === itemGrid.model) {
                itemGrid.model.addFavorite(kicker.dragSource.favoriteId, itemGrid.model.dropPlaceholderIndex);
                gridView.currentIndex = -1;
            }
        }

        Timer {
            id: resetAnimationDurationTimer

            interval: 120
            repeat: false

            onTriggered: {
                gridView.animationDuration = interval - 20;
            }
        }

        PlasmaComponents.ScrollView {
            id: scrollArea

            anchors.fill: parent

            focus: true

            PlasmaComponents.ScrollBar.horizontal.policy: itemGrid.horizontalScrollBarPolicy

            GridView {
                id: gridView

                signal itemContainsMouseChanged(bool containsMouse)

                property int iconSize: Kirigami.Units.iconSizes.huge

                property bool animating: false
                property int animationDuration: itemGrid.dropEnabled ? resetAnimationDurationTimer.interval : 0

                focus: true
                clip: height < contentHeight + topMargin + bottomMargin
                currentIndex: -1

                move: Transition {
                    enabled: itemGrid.dropEnabled

                    SequentialAnimation {
                        PropertyAction { target: gridView; property: "animating"; value: true }

                        NumberAnimation {
                            duration: gridView.animationDuration
                            properties: "x, y"
                            easing.type: Easing.OutQuad
                        }

                        PropertyAction { target: gridView; property: "animating"; value: false }
                    }
                }

                moveDisplaced: Transition {
                    enabled: itemGrid.dropEnabled

                    SequentialAnimation {
                        PropertyAction { target: gridView; property: "animating"; value: true }

                        NumberAnimation {
                            duration: gridView.animationDuration
                            properties: "x, y"
                            easing.type: Easing.OutQuad
                        }

                        PropertyAction { target: gridView; property: "animating"; value: false }
                    }
                }

                keyNavigationWraps: false
                boundsBehavior: Flickable.StopAtBounds

                delegate: ItemGridDelegate {
                    showLabel: itemGrid.showLabels
                }

                highlight: Item {
                    property bool isDropPlaceHolder: "dropPlaceholderIndex" in itemGrid.model && itemGrid.currentIndex === itemGrid.model.dropPlaceholderIndex

                    PlasmaExtras.Highlight {
                        visible: gridView.currentItem && !isDropPlaceHolder
                        hovered: true
                        pressed: hoverArea.pressed

                        anchors.fill: parent
                    }

                    KSvg.FrameSvgItem {
                        visible: gridView.currentItem && isDropPlaceHolder

                        anchors.fill: parent

                        imagePath: "widgets/viewitem"
                        prefix: "selected"

                        opacity: 0.5

                        Kirigami.Icon {
                            anchors {
                                right: parent.right
                                rightMargin: parent.margins.right
                                bottom: parent.bottom
                                bottomMargin: parent.margins.bottom
                            }

                            width: Kirigami.Units.iconSizes.smallMedium
                            height: width

                            source: "list-add"
                            active: false
                        }
                    }
                }

                highlightFollowsCurrentItem: true
                highlightMoveDuration: 0

                onCurrentIndexChanged: {
                    if (currentIndex !== -1) {
                        hoverArea.hoverEnabled = false
                        focus = true;
                    }
                }

                onCountChanged: {
                    animationDuration = 0;
                    resetAnimationDurationTimer.start();
                }

                onModelChanged: {
                    currentIndex = -1;
                }

                Keys.onLeftPressed: event => {
                    if (itemGrid.currentCol() !== 0) {
                        event.accepted = true;
                        moveCurrentIndexLeft();
                    } else {
                        itemGrid.keyNavLeft();
                    }
                }

                Keys.onRightPressed: event => {
                    var columns = Math.floor(width / cellWidth);

                    if (itemGrid.currentCol() !== columns - 1 && currentIndex !== count -1) {
                        event.accepted = true;
                        moveCurrentIndexRight();
                    } else {
                        itemGrid.keyNavRight();
                    }
                }

                Keys.onUpPressed: event => {
                    if (itemGrid.currentRow() !== 0) {
                        event.accepted = true;
                        moveCurrentIndexUp();
                        positionViewAtIndex(currentIndex, GridView.Contain);
                    } else {
                        itemGrid.keyNavUp();
                    }
                }

                Keys.onDownPressed: event => {
                    if (itemGrid.currentRow() < itemGrid.lastRow()) {
                        // Fix moveCurrentIndexDown()'s lack of proper spatial nav down
                        // into partial columns.
                        event.accepted = true;
                        var columns = Math.floor(width / cellWidth);
                        var newIndex = currentIndex + columns;
                        currentIndex = Math.min(newIndex, count - 1);
                        positionViewAtIndex(currentIndex, GridView.Contain);
                    } else {
                        itemGrid.keyNavDown();
                    }
                }

                onItemContainsMouseChanged: containsMouse => {
                    if (!containsMouse) {
                        if (!actionMenu.opened) {
                            gridView.currentIndex = -1;
                        }

                        hoverArea.pressX = -1;
                        hoverArea.pressY = -1;
                        hoverArea.lastX = -1;
                        hoverArea.lastY = -1;
                        hoverArea.pressedItem = null;
                        hoverArea.hoverEnabled = true;
                    }
                }
            }
        }

        MouseArea {
            id: hoverArea

            anchors.fill: parent

            property int pressX: -1
            property int pressY: -1
            property int lastX: -1
            property int lastY: -1
            property Item pressedItem: null

            acceptedButtons: Qt.LeftButton | Qt.RightButton

            hoverEnabled: true

            function updatePositionProperties(x, y) {
                // Prevent hover event synthesis in QQuickWindow interfering
                // with keyboard navigation by ignoring repeated events with
                // identical coordinates. As the work done here would be re-
                // dundant in any case, these are safe to ignore.
                if (lastX === x && lastY === y) {
                    return;
                }

                lastX = x;
                lastY = y;

                var cPos = mapToItem(gridView.contentItem, x, y);
                var item = gridView.itemAt(cPos.x, cPos.y);

                if (!item) {
                    gridView.currentIndex = -1;
                    pressedItem = null;
                } else {
                    itemGrid.focus = (item.itemIndex !== -1)
                    gridView.currentIndex = item.itemIndex;
                }

                return item;
            }

            onPressed: mouse => {
                mouse.accepted = true;

                updatePositionProperties(mouse.x, mouse.y);

                pressX = mouse.x;
                pressY = mouse.y;

                if (mouse.button === Qt.RightButton) {
                    if (gridView.currentItem) {
                        if (gridView.currentItem.hasActionList) {
                            var mapped = mapToItem(gridView.currentItem, mouse.x, mouse.y);
                            gridView.currentItem.openActionMenu(mapped.x, mapped.y);
                        }
                    } else {
                        var mapped = mapToItem(rootItem, mouse.x, mouse.y);
                        contextMenu.open(mapped.x, mapped.y);
                    }
                } else {
                    pressedItem = gridView.currentItem;
                }
            }

            onReleased: mouse => {
                mouse.accepted = true;
                updatePositionProperties(mouse.x, mouse.y);

                if (!dragHelper.dragging) {
                    if (pressedItem) {
                        if ("trigger" in gridView.model) {
                            gridView.model.trigger(pressedItem.itemIndex, "", null);
                            root.toggle();
                        }

                        itemGrid.itemActivated(pressedItem.itemIndex, "", null);
                    } else if (mouse.button === Qt.LeftButton) {
                        root.toggle();
                    }
                }

                pressX = pressY = -1;
                pressedItem = null;
            }

            onPositionChanged: mouse => {
                var item = pressedItem ? pressedItem : updatePositionProperties(mouse.x, mouse.y);

                if (gridView.currentIndex !== -1) {
                    if (itemGrid.dragEnabled && pressX !== -1 && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                        if ("pluginName" in item.m) {
                            dragHelper.startDrag(kicker, item.url, item.icon,
                            "text/x-plasmoidservicename", item.m.pluginName);
                        } else {
                            dragHelper.startDrag(kicker, item.url, item.icon);
                        }

                        kicker.dragSource = item;

                        pressX = -1;
                        pressY = -1;
                    }
                }
            }
        }
    }
}
