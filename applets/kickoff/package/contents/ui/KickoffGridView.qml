/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                        *
 *   Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>             *
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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

FocusScope {
    id: itemGrid

    function keyNavLeft() {
        if (itemGrid.currentCol() !== 0) {
            if (LayoutMirroring.enabled) {
                gridView.moveCurrentIndexRight();
            } else {
                gridView.moveCurrentIndexLeft();
            }
            return true;
        } else {
            return false;
        }
    }

    function keyNavRight() {
        if (itemGrid.currentCol() !== columns - 1 && gridView.currentIndex != gridView.count -1) {
            if (LayoutMirroring.enabled) {
                gridView.moveCurrentIndexLeft();
            } else {
                gridView.moveCurrentIndexRight();
            }
            return true;
        } else {
            return false;
        }
    }

    function keyNavUp() {
        if (itemGrid.currentRow() !== 0) {
            gridView.moveCurrentIndexUp();
            gridView.positionViewAtIndex(gridView.currentIndex, GridView.Contain);
            return true;
        } else {
            return false;
        }
    }

    function keyNavDown() {
        if (itemGrid.currentRow() < itemGrid.lastRow()) {
            //Fix moveCurrentIndexDown()'s lack of proper spatial nav down
            //into partial columns.
            var newIndex = gridView.currentIndex + columns;
            gridView.currentIndex = Math.min(newIndex, gridView.count - 1);
            gridView.positionViewAtIndex(gridView.currentIndex, GridView.Contain);
            return true;
        } else {
            return false;
        }
    }

    readonly property Item gridView: gridView

    property alias currentIndex: gridView.currentIndex
    property alias currentItem: gridView.currentItem
    property alias contentItem: gridView.contentItem
    property alias count: gridView.count
    property alias model: gridView.model

    property alias cellWidth: gridView.cellWidth
    property alias cellHeight: gridView.cellHeight
    property alias iconSize: gridView.iconSize

    property alias contentHeight: gridView.contentHeight
    property alias interactive: gridView.interactive
    property alias move: gridView.move
    property alias moveDisplaced: gridView.moveDisplaced

    // NOTE: We don't name panes because doing so would be annoying and redundant
    Accessible.role: Accessible.Paragraph
    Accessible.name: i18n("Grid with %1 rows, %2 columns", rows, columns) // can't use i18np here

    onFocusChanged: {
        if (!focus) {
            currentIndex = 0;
        }
    }

    Connections {
        target: plasmoid

        function onExpandedChanged() {
            if (!plasmoid.expanded) {
                gridView.positionAtBeginning();
            }
        }
    }

    property int columns: Math.floor(width / itemGrid.cellWidth);
    property int rows: Math.ceil(count / columns);

    function currentRow() {
        if (currentIndex == -1) {
            return -1;
        }

        return Math.floor(currentIndex / Math.floor(width / itemGrid.cellWidth));
    }

    function currentCol() {
        if (currentIndex == -1) {
            return -1;
        }

        return currentIndex - (currentRow() * Math.floor(width / itemGrid.cellWidth));
    }

    function lastRow() {
        return Math.ceil(count / columns) - 1;
    }

    function tryActivate(row, col) {
        if (count) {
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

        onActionClicked: {
            visualParent.actionTriggered(actionId, actionArgument);
        }
    }

    PC3.ScrollView {
        anchors.fill: parent
        PC3.ScrollBar.horizontal.visible: false
        focus: true

        GridView {
            id: gridView
            property int iconSize: PlasmaCore.Units.iconSizes.large

            property bool animating: false
            cellWidth: gridView.iconSize + PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height
                + (4 * PlasmaCore.Units.smallSpacing + Math.round(PlasmaCore.Units.smallSpacing * 1.5)) //item margins + spacing
                + (2 * gridView.cellMargin) //highlight padding
                + cellMargin * 2 //actual margins
            cellHeight: cellWidth

            property int cellMargin: Math.round(PlasmaCore.Units.smallSpacing * 1.5)

            leftMargin: LayoutMirroring.enabled ? 0 : Math.round((parent.width-cellWidth*Math.floor(parent.width / cellWidth))/2)
            rightMargin: LayoutMirroring.enabled ? Math.round((parent.width-cellWidth*Math.floor(parent.width / cellWidth))/2) : 0
            topMargin: Math.round(PlasmaCore.Units.smallSpacing * 0.25)
            bottomMargin: (contentHeight + topMargin) > parent.height ? Math.round(PlasmaCore.Units.smallSpacing * 0.25) : 0
            focus: true

            // Let root handle keyboard interaction
            Keys.forwardTo: [root]

            currentIndex: 0
            function positionAtBeginning() {
                positionViewAtBeginning();
                // positionViewAtBeginning doesn't account for margins
                // move content manually only if it overflows
                if (visibleArea.heightRatio !== 1.0) {
                    contentY -= topMargin;
                }
                currentIndex = 0;
            }
            keyNavigationWraps: false
            boundsBehavior: Flickable.StopAtBounds

            delegate: KickoffGridItem { }

            PC3.ToolTip {
                parent: gridView.currentItem ? gridView.currentItem : gridView
                visible: itemGrid.parent == root.currentContentView && gridView.currentItem ? gridView.currentItem.labelTruncated && ((navigationMethod.state == "keyboard" && keyboardNavigation.state == "RightColumn" && itemGrid.activeFocus) || hoverArea.containsMouse) : false
                text: gridView.currentItem ? gridView.currentItem.display : ""
            }

            highlight: Item {
                opacity: navigationMethod.state != "keyboard" || (keyboardNavigation.state == "RightColumn" && gridView.activeFocus) ? 1 : 0.5
                PlasmaCore.FrameSvgItem {
                    visible: gridView.currentItem

                    anchors.fill: parent
                    anchors.margins: gridView.cellMargin

                    imagePath: "widgets/viewitem"
                    prefix: "hover"
                }
            }

            highlightFollowsCurrentItem: true
            highlightMoveDuration: 0

            onModelChanged: {
                currentIndex = 0;
            }

            MouseArea {
                anchors.left: parent.left

                width: parent.width
                height: parent.height

                id: hoverArea

                property Item pressed: null
                property int pressX: -1
                property int pressY: -1
                property bool tapAndHold: false

                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onPressed: {
                    var mapped = gridView.mapToItem(gridView.contentItem, mouse.x, mouse.y);
                    var item = gridView.itemAt(mapped.x, mapped.y);

                    if (!item) {
                        return;
                    }

                    if (mouse.buttons & Qt.RightButton) {
                        if (item.hasActionList) {
                            mapped = gridView.contentItem.mapToItem(item, mapped.x, mapped.y);
                            gridView.currentItem.openActionMenu(mapped.x, mapped.y);
                        }
                    } else {
                        pressed = item;
                        pressX = mouse.x;
                        pressY = mouse.y;
                    }
                }

                onReleased: {
                    var mapped = gridView.mapToItem(gridView.contentItem, mouse.x, mouse.y);
                    var item = gridView.itemAt(mapped.x, mapped.y);

                    if (item && pressed === item && !tapAndHold) {
                        if (item.appView) {
                            if (mouse.source == Qt.MouseEventSynthesizedByQt) {
                                positionChanged(mouse);
                            }
                            if (isManagerMode && view.currentItem && !view.currentItem.modelChildren) {
                                view.activatedItem = view.currentItem;
                                view.moveRight();
                            } else if (!isManagerMode) {
                                view.state = "OutgoingLeft";
                            }
                        } else {
                            item.activate();
                        }
                    }
                    if (tapAndHold && mouse.source == Qt.MouseEventSynthesizedByQt) {
                        if (item.hasActionList) {
                            mapped = gridView.contentItem.mapToItem(item, mapped.x, mapped.y);
                            gridView.currentItem.openActionMenu(mapped.x, mapped.y);
                        }
                    }
                    pressed = null;
                    pressX = -1;
                    pressY = -1;
                    tapAndHold = false;
                }

                onPositionChanged: {
                    var mapped = gridView.mapToItem(gridView.contentItem, mouse.x, mouse.y);
                    var item = gridView.itemAt(mapped.x, mapped.y);

                    if (item) {
                        navigationMethod.state = "mouse"
                        keyboardNavigation.state = "RightColumn"
                        if (kickoff.dragSource == null || kickoff.dragSource == item) {
                            gridView.currentIndex = item.itemIndex;
                        }
                    }

                    if (mouse.source != Qt.MouseEventSynthesizedByQt || tapAndHold) {
                        if (pressed && pressX != -1 && pressed.url && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                            kickoff.dragSource = item;
                            if (mouse.source == Qt.MouseEventSynthesizedByQt) {
                                dragHelper.dragIconSize = PlasmaCore.Units.iconSizes.huge
                                dragHelper.startDrag(kickoff, pressed.url, pressed.decoration);
                            } else {
                                dragHelper.dragIconSize = PlasmaCore.Units.iconSizes.medium
                                dragHelper.startDrag(kickoff, pressed.url, pressed.decoration);
                            }
                            pressed = null;
                            pressX = -1;
                            pressY = -1;
                            tapAndHold = false;
                        }
                    }
                }

                onContainsMouseChanged: {
                    if (!containsMouse) {
                        pressed = null;
                        pressX = -1;
                        pressY = -1;
                        tapAndHold = false;
                    }
                }

                onPressAndHold: {
                    if (mouse.source == Qt.MouseEventSynthesizedByQt) {
                        tapAndHold = true;
                        positionChanged(mouse);
                    }
                }
            }
        }
    }
}
