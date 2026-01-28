/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.private.kicker as Kicker

PlasmaComponents.ScrollView {
    id: itemMultiGrid

    signal interactionConcluded()

    anchors {
        top: parent.top
    }

    width: parent.width

    implicitHeight: itemColumn.implicitHeight

    signal keyNavLeft(int subGridIndex)
    signal keyNavRight(int subGridIndex)
    signal keyNavUp()
    signal keyNavDown()

    property bool grabFocus: false

    property int cellSize
    property int iconSize
    property alias model: repeater.model
    property alias count: repeater.count
    property alias flickableItem: flickable
    property var firstGrid

    function subGridAt(index) {
        let subgrid = repeater.itemAt(index)
        if (subgrid) {
            return subgrid.itemGrid
        }
    }

    function selectFirstElement() {
        let foundAny = false
        for (var i = 0; i < repeater.count; i++) {
            let grid = subGridAt(i)
            if (grid && grid.count > 0) {
                if (!foundAny) {
                    itemMultiGrid.firstGrid = grid
                    grid.currentIndex = 0
                    if (itemMultiGrid.grabFocus) {
                        grid.focus = true
                    }
                    foundAny = true
                } else {
                    grid.currentIndex = -1
                }
            }
        }
    }

    function tryActivate(row, col) { // FIXME TODO: Cleanup messy algo.
        if (flickable.contentY > 0) {
            row = 0;
        }

        var target = null;
        var rows = 0;

        for (var i = 0; i < repeater.count; i++) {
            var grid = subGridAt(i);

            if (rows <= row) {
                target = grid;
                rows += grid.lastRow() + 2; // Header counts as one.
            } else {
                break;
            }
        }

        if (target) {
            rows -= (target.lastRow() + 2);
            target.tryActivate(row - rows, col);
        }
    }

    onFocusChanged: {
        if (!focus) {
            for (var i = 0; i < repeater.count; i++) {
                subGridAt(i).focus = false;
            }
        }
    }

    Flickable {
        id: flickable

        flickableDirection: Flickable.VerticalFlick
        contentHeight: itemColumn.implicitHeight
        //focusPolicy: Qt.NoFocus

        Column {
            id: itemColumn

            width: itemMultiGrid.width - Kirigami.Units.gridUnit

            Repeater {
                id: repeater

                delegate: Item {
                    id: gridDelegate

                    required property int index

                    width: itemColumn.width - Kirigami.Units.gridUnit
                    height: headerHeight + gridView.height + (index == repeater.count - 1 ? 0 : footerHeight)

                    property int headerHeight: (gridViewLabel.height
                        + gridViewLabelUnderline.height + Kirigami.Units.gridUnit)
                    property int footerHeight: (Math.ceil(headerHeight / itemMultiGrid.cellSize) * itemMultiGrid.cellSize) - headerHeight

                    property Item itemGrid: gridView
                    visible: gridView.count > 0

                    Kirigami.Heading {
                        id: gridViewLabel

                        anchors.top: parent.top

                        x: Kirigami.Units.smallSpacing
                        width: parent.width - x
                        height: dummyHeading.height

                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                        opacity: 1.0

                        color: "white" // FIXME TODO: Respect theming?

                        level: 1

                        text: repeater.model.modelForRow(gridDelegate.index).description
                        textFormat: Text.PlainText
                    }

                    KSvg.SvgItem {
                        id: gridViewLabelUnderline

                        anchors.top: gridViewLabel.bottom

                        width: parent.width - Kirigami.Units.gridUnit

                        imagePath: "widgets/line"
                        elementId: "horizontal-line"
                    }

                    MouseArea {
                        width: parent.width
                        height: parent.height

                        onClicked: itemMultiGrid.interactionConcluded()
                    }

                    ItemGridView {
                        id: gridView

                        anchors {
                            top: gridViewLabelUnderline.bottom
                            topMargin: Kirigami.Units.gridUnit
                        }

                        width: parent.width
                        height: Math.ceil(count / Math.floor(width / itemMultiGrid.cellSize)) * itemMultiGrid.cellSize

                        cellWidth: itemMultiGrid.cellSize
                        cellHeight: itemMultiGrid.cellSize
                        iconSize: itemMultiGrid.iconSize
                        hoverEnabled: itemMultiGrid.hoverEnabled

                        verticalScrollBarPolicy: PlasmaComponents.ScrollBar.AlwaysOff

                        model: repeater.model.modelForRow(gridDelegate.index)

                        onFocusChanged: {
                            if (focus) {
                                itemMultiGrid.focus = true;
                            }
                        }

                        onInteractionConcluded: itemMultiGrid.interactionConcluded()
                        onCountChanged: itemMultiGrid.selectFirstElement()

                        onCurrentItemChanged: {
                            if (!currentItem) {
                                return;
                            }

                            if (gridDelegate.index == 0 && currentRow() === 0) {
                                flickable.contentY = 0;
                                return;
                            }

                            var y = currentItem.y;
                            y = contentItem.mapToItem(flickable.contentItem, 0, y).y;

                            if (y < flickable.contentY) {
                                flickable.contentY = y;
                            } else {
                                y += itemMultiGrid.cellSize;
                                y -= flickable.contentY;
                                y -= itemMultiGrid.height;

                                if (y > 0) {
                                    flickable.contentY += y;
                                }
                            }
                        }

                        onKeyNavLeft: {
                            itemMultiGrid.keyNavLeft(gridDelegate.index);
                            currentIndex = -1
                        }

                        onKeyNavRight: {
                            itemMultiGrid.keyNavRight(gridDelegate.index);
                            currentIndex = -1
                        }

                        onKeyNavUp: {
                            if (gridDelegate.index > 0) {
                                for (var i = gridDelegate.index - 1; i >= 0; i--) {
                                    if (itemMultiGrid.subGridAt(i).count > 0) {
                                        itemMultiGrid.subGridAt(i).tryActivate(itemMultiGrid.subGridAt(i).lastRow(), currentCol());
                                        break;
                                    }
                                }
                            } else {
                                itemMultiGrid.keyNavUp();
                            }
                            currentIndex = -1
                        }

                        onKeyNavDown: {
                            if (gridDelegate.index < repeater.count - 1) {
                                for (var i = gridDelegate.index + 1; i < repeater.count; i++) {
                                    if (itemMultiGrid.subGridAt(i).count > 0) {
                                        itemMultiGrid.subGridAt(i).tryActivate(0, currentCol());
                                        break;
                                    }
                                }
                            } else {
                                itemMultiGrid.keyNavDown();
                            }
                            currentIndex = -1
                        }
                        Keys.onBacktabPressed: event => {
                            currentIndex = -1
                            event.accepted = false // pass to mainColumn handler
                        }
                    }

                    // HACK: Steal wheel events from the nested grid view and forward them to
                    // the ScrollView's internal WheelArea.
                    Kicker.WheelInterceptor {
                        anchors.fill: gridView
                        z: 1

                        destination: findWheelArea(itemMultiGrid)
                    }
                }
            }
        }
    }
}
