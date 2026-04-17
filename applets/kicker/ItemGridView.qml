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

    property alias dropEnabled: dropAreaLoader.active
    property bool hoverEnabled: true
    property GridView view: gridView

    property alias currentIndex: gridView.currentIndex
    property alias currentItem: gridView.currentItem
    property alias contentItem: gridView.contentItem
    property alias count: gridView.count
    property alias model: gridView.model

    property alias cellWidth: gridView.cellWidth
    property alias cellHeight: gridView.cellHeight
    property alias iconSize: gridView.iconSize

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

    Loader {
        id: dropAreaLoader
        anchors.fill: parent
        active: false
        onActiveChanged: {
            if (!active && "dropPlaceHolderIndex" in model) {
                model.dropPlaceHolderIndex = -1;
            }
        }

        sourceComponent : DropArea {
            id: dropArea

            anchors.fill: parent

            onPositionChanged: event => {
                let draggedItem = drag.source as ItemGridDelegate
                if (gridView.animating || !draggedItem) {
                    return;
                }

                var x = Math.max(0, event.x - (width % itemGrid.cellWidth));
                var cPos = mapToItem(gridView.contentItem, x, event.y);
                var item = gridView.itemAt(cPos.x, cPos.y) as ItemGridDelegate;

                if (item) {
                    if (draggedItem.parent === gridView.contentItem) {
                        if (item !== draggedItem) {
                            item.GridView.view.model.moveRow(draggedItem.itemIndex, item.itemIndex);
                        }
                    } else if (draggedItem.favoritesModel === itemGrid.model
                        && !itemGrid.model.isFavorite(draggedItem.favoriteId)) {
                        var hasPlaceholder = (itemGrid.model.dropPlaceholderIndex !== -1);

                        itemGrid.model.dropPlaceholderIndex = item.itemIndex;

                        if (!hasPlaceholder) {
                            gridView.currentIndex = (item.itemIndex - 1);
                        }
                    }
                } else if (draggedItem.parent !== gridView.contentItem
                    && draggedItem.favoritesModel === itemGrid.model
                    && !itemGrid.model.isFavorite(draggedItem.favoriteId)) {
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
                let draggedItem = drag.source as ItemGridDelegate
                if (draggedItem && draggedItem.parent !== gridView.contentItem &&  draggedItem.favoritesModel === itemGrid.model) {
                    itemGrid.model.addFavorite(draggedItem.favoriteId, itemGrid.model.dropPlaceholderIndex);
                    gridView.currentIndex = -1;
                    drop.accept(Qt.CopyAction)
                } else if (draggedItem && draggedItem.parent !== gridView.contentItem && draggedItem.favoritesModel.isFavorite(draggedItem.favoriteId)) {
                    draggedItem.showUnfavoritePlaceholder = true
                    draggedItem.favoritesModel.removeFavorite(draggedItem.favoriteId)
                    drop.accept(Qt.MoveAction)
                } else if (draggedItem && draggedItem.parent === gridView.contentItem) {
                    drop.accept(Qt.MoveAction)
                }
            }
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

        PlasmaComponents.ScrollBar.horizontal.policy: PlasmaComponents.ScrollBar.AlwaysOff

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
                iconSize: gridView.iconSize
                onInteractionConcluded: itemGrid.interactionConcluded()
                hoverEnabled: itemGrid.hoverEnabled
                onHoveredChanged: {
                    if (hovered && !ActionMenu.opened) {
                        gridView.currentIndex = index
                    } else if (GridView.isCurrentItem && !ActionMenu.opened) {
                            gridView.currentIndex = -1;
                    }
                }
                isDraggableFavorite: itemGrid.dropEnabled
                showUnfavoritePlaceholder: Drag.active && itemGrid.dropEnabled && !(dropAreaLoader.item as DropArea).containsDrag
            }

            Connections {
                target: ActionMenu
                enabled: !!gridView.currentItem
                function onClosed() {
                    if ((!gridView.currentItem as ItemGridDelegate)?.hovered) {
                        gridView.currentIndex = -1;
                    }
                }
            }

            highlight: Item {
                id: highlightItem
                property bool isDropPlaceHolder: "dropPlaceholderIndex" in itemGrid.model && itemGrid.currentIndex === itemGrid.model.dropPlaceholderIndex

                PlasmaExtras.Highlight {
                    visible: gridView.currentItem && !highlightItem.isDropPlaceHolder
                    hovered: true
                    pressed: (gridView.currentItem as ItemGridDelegate)?.pressed ?? false

                    anchors.fill: parent
                }

                KSvg.FrameSvgItem {
                    visible: gridView.currentItem && highlightItem.isDropPlaceHolder

                    anchors.fill: parent

                    imagePath: "widgets/viewitem"
                    prefix: "selected"

                    opacity: 0.5

                    Kirigami.Icon {
                        anchors.centerIn: parent

                        width: gridView.iconSize
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
            TapHandler {
                onTapped: itemGrid.interactionConcluded()
            }
        }
    }
}
