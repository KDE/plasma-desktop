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

FocusScope {
    id: itemGrid

    signal keyNavLeft
    signal keyNavRight
    signal keyNavUp
    signal keyNavDown

    signal interactionConcluded()

    property bool dragEnabled: true
    property bool dropEnabled: false
    property bool showLabels: true
    property bool hoverEnabled: true
    property var view: gridView

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
            var item = gridView.itemAt(cPos.x, cPos.y) as ItemGridDelegate;

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
                    iconSize: gridView.iconSize
                    onInteractionConcluded: itemGrid.interactionConcluded()
                    hoverEnabled: itemGrid.hoverEnabled
                    onHoveredChanged: {
                        if (hovered) {
                            gridView.currentIndex = index
                        } else {
                            if (!actionMenu.opened) {
                                gridView.currentIndex = -1;
                            }

                            hoverArea.pressX = -1;
                            hoverArea.pressY = -1;
                            hoverArea.lastX = -1;
                            hoverArea.lastY = -1;
                            hoverArea.pressedItem = null;
                            hoverArea.hoverEnabled = itemGrid.hoverEnabled;
                        }
                    }
                }

                highlight: Item {
                    id: highlightItem
                    property bool isDropPlaceHolder: "dropPlaceholderIndex" in itemGrid.model && itemGrid.currentIndex === itemGrid.model.dropPlaceholderIndex

                    PlasmaExtras.Highlight {
                        visible: gridView.currentItem && !highlightItem.isDropPlaceHolder
                        hovered: true
                        pressed: hoverArea.pressed

                        anchors.fill: parent
                    }

                    KSvg.FrameSvgItem {
                        visible: gridView.currentItem && highlightItem.isDropPlaceHolder

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

                function handleLeftRightArrow(event: KeyEvent) : void {
                    let backArrowKey = (event.key === Qt.Key_Left && Application.layoutDirection === Qt.LeftToRight) ||
                        (event.key === Qt.Key_Right && Application.layoutDirection === Qt.RightToLeft)
                    let forwardArrowKey = (event.key === Qt.Key_Right && Application.layoutDirection === Qt.LeftToRight) ||
                        (event.key === Qt.Key_Left && Application.layoutDirection === Qt.RightToLeft)

                    if (backArrowKey) {
                        if (itemGrid.currentCol() !== 0) {
                            // GridView move..() already handles RtL
                            (event.key === Qt.Key_Left) ? moveCurrentIndexLeft() : moveCurrentIndexRight();
                        } else {
                            itemGrid.keyNavLeft();
                        }
                    } else if (forwardArrowKey) {
                        var columns = Math.floor(width / cellWidth);

                        if (itemGrid.currentCol() !== columns - 1 && currentIndex !== count -1) {
                            // GridView move..() already handles RtL
                            (event.key === Qt.Key_Left) ? moveCurrentIndexLeft() : moveCurrentIndexRight()
                        } else {
                            itemGrid.keyNavRight();
                        }
                    }
                }

                Keys.onLeftPressed: event => handleLeftRightArrow(event)
                Keys.onRightPressed: event => handleLeftRightArrow(event)
                Keys.onEnterPressed: Keys.returnPressed()
                Keys.onReturnPressed: {
                    if (gridView.model.trigger) {
                        gridView.model.trigger(currentIndex, "", null)
                        itemGrid.interactionConcluded()
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
            }
        }

        MouseArea {
            id: hoverArea

            anchors.fill: parent
            anchors.rightMargin: scrollArea.effectiveScrollBarWidth

            property int pressX: -1
            property int pressY: -1
            property int lastX: -1
            property int lastY: -1
            property ItemGridDelegate pressedItem: null

            acceptedButtons: Qt.LeftButton | Qt.RightButton

            hoverEnabled: itemGrid.hoverEnabled

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
                var item = gridView.itemAt(cPos.x, cPos.y) as ItemGridDelegate;

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

                const currentDelegate = gridView.currentItem as ItemGridDelegate

                if (mouse.button === Qt.RightButton) {
                    if (currentDelegate) {
                        if (currentDelegate.hasActionList) {
                            var mapped = mapToItem(currentDelegate, mouse.x, mouse.y);
                            currentDelegate.openActionMenu(mapped.x, mapped.y);
                        }
                    } else {
                        var mapped = mapToItem(rootItem, mouse.x, mouse.y);
                        contextMenu.open(mapped.x, mapped.y);
                    }
                } else {
                    pressedItem = currentDelegate;
                }
            }

            onReleased: mouse => {
                mouse.accepted = true;
                updatePositionProperties(mouse.x, mouse.y);

                if (!dragHelper.dragging) {
                    if (pressedItem) {
                        if ("trigger" in gridView.model) {
                            gridView.model.trigger(pressedItem.itemIndex, "", null);
                            itemGrid.interactionConcluded()
                        }
                    } else if (mouse.button === Qt.LeftButton) {
                        itemGrid.interactionConcluded()
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
                            dragHelper.startDrag(kicker, item.url, item.decoration,
                                "text/x-plasmoidservicename", item.model.pluginName);
                        } else {
                            dragHelper.startDrag(kicker, item.url, item.decoration);
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
