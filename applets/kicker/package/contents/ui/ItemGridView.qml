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

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0
import org.kde.draganddrop 2.0

FocusScope {
    id: itemGrid

    signal keyNavLeft
    signal keyNavRight
    signal keyNavUp
    signal keyNavDown

    property bool dragEnabled: false
    property bool showLabels: true

    property int pressX: -1
    property int pressY: -1

    property alias currentIndex: gridView.currentIndex
    property alias currentItem: gridView.currentItem
    property alias contentItem: gridView.contentItem
    property alias count: gridView.count
    property alias model: gridView.model

    property alias cellWidth: gridView.cellWidth
    property alias cellHeight: gridView.cellHeight

    property alias horizontalScrollBarPolicy: scrollArea.horizontalScrollBarPolicy
    property alias verticalScrollBarPolicy: scrollArea.verticalScrollBarPolicy

    onFocusChanged: {
        if (!focus) {
            currentIndex = -1;
        }
    }

    function currentRow() {
        if (currentIndex == -1) {
            return -1;
        }

        return Math.floor(currentIndex / Math.floor(width / cellWidth));
    }

    function currentCol() {
        if (currentIndex == -1) {
            return -1;
        }

        return currentIndex - (currentRow() * Math.floor(width / cellWidth));
    }

    function lastRow() {
        var columns = Math.floor(width / cellWidth);
        return Math.ceil(count / columns) - 1;
    }

    function tryActivate(row, col) {
        if (count) {
            var columns = Math.floor(width / cellWidth);
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

    DropArea {
        id: dropArea

        anchors.fill: parent

        onDragMove: {
            if (!dragEnabled || gridView.animating) {
                return;
            }

            var cPos = mapToItem(gridView.contentItem, event.x, event.y);
            var item = gridView.itemAt(cPos.x, cPos.y);

            if (item && item != kicker.dragSource && kicker.dragSource.parent == gridView.contentItem) {
                item.GridView.view.model.moveRow(dragSource.itemIndex, item.itemIndex);
            }

        }

        MouseEventListener {
            anchors.fill: parent

            hoverEnabled: true

            onPressed: {
                pressX = mouse.x;
                pressY = mouse.y;
            }

            onReleased: {
                pressX = -1;
                pressY = -1;
            }

            onClicked: {
                var cPos = mapToItem(gridView.contentItem, mouse.x, mouse.y);
                var item = gridView.itemAt(cPos.x, cPos.y);

                if (!item) {
                    root.toggle();
                }
            }

            onPositionChanged: {
                var cPos = mapToItem(gridView.contentItem, mouse.x, mouse.y);
                var item = gridView.itemAt(cPos.x, cPos.y);

                if (!item) {
                    gridView.currentIndex = -1;
                } else {
                    gridView.currentIndex = item.itemIndex;
                    itemGrid.focus = (currentIndex != -1)

                    if (dragEnabled && pressX != -1 && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                        kicker.dragSource = item;
                        dragHelper.startDrag(kicker, item.url);
                        pressX = -1;
                        pressY = -1;
                    }
                }
            }

            onContainsMouseChanged: {
                if (!containsMouse) {
                    gridView.currentIndex = -1;
                    pressX = -1;
                    pressY = -1;
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

            PlasmaExtras.ScrollArea {
                id: scrollArea

                anchors.fill: parent

                focus: true

                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

                GridView {
                    id: gridView

                    property bool animating: false
                    property int animationDuration: dragEnabled ? resetAnimationDurationTimer.interval : 0

                    focus: true

                    currentIndex: -1

                    move: Transition {
                        enabled: itemGrid.dragEnabled

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
                        enabled: itemGrid.dragEnabled

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
                        showLabel: showLabels
                    }

                    highlight: PlasmaComponents.Highlight {}
                    highlightFollowsCurrentItem: true
                    highlightMoveDuration: 0

                    onCurrentIndexChanged: {
                        if (currentIndex != -1) {
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

                    Keys.onLeftPressed: {
                        if (currentCol() != 0) {
                            event.accepted = true;
                            moveCurrentIndexLeft();
                        } else {
                            itemGrid.keyNavLeft();
                        }
                    }

                    Keys.onRightPressed: {
                        var columns = Math.floor(width / cellWidth);

                        if (currentCol() != columns - 1 && currentIndex != count -1) {
                            event.accepted = true;
                            moveCurrentIndexRight();
                        } else {
                            itemGrid.keyNavRight();
                        }
                    }

                    Keys.onUpPressed: {
                        if (currentRow() != 0) {
                            event.accepted = true;
                            moveCurrentIndexUp();
                            positionViewAtIndex(currentIndex, GridView.Contain);
                        } else {
                            itemGrid.keyNavUp();
                        }
                    }

                    Keys.onDownPressed: {
                        if (currentRow() < itemGrid.lastRow()) {
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
        }
    }
}
