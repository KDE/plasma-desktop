/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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

import QtQuick 2.2
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

import org.kde.plasma.private.folder 0.1 as Folder

import "../code/tools.js" as FolderTools

FocusScope {
    id: main

    property alias isRootView: gridView.isRootView
    property alias url: dir.url
    property alias errorString: dir.errorString
    property alias filterMode: dir.filterMode
    property alias filterPattern: dir.filterPattern
    property alias filterMimeTypes: dir.filterMimeTypes
    property alias flow: gridView.flow
    property alias layoutDirection: gridView.layoutDirection
    property alias cellWidth: gridView.cellWidth
    property alias cellHeight: gridView.cellHeight

    function linkHere(sourceUrl) {
        dir.linkHere(sourceUrl);
    }

    onFocusChanged: {
        if (!focus) {
            dir.clearSelection();
        }
    }

    MouseEventListener {
        id: listener

        anchors.fill: parent

        property alias hoveredItem: gridView.hoveredItem

        property bool doubleClickInProgress: false

        acceptedButtons: (hoveredItem == null) ? Qt.LeftButton : (Qt.LeftButton | Qt.RightButton)
        hoverEnabled: true

        onPressed: {
            gridView.focus = true;
            main.focus = true;

            if (childAt(mouse.x, mouse.y) != editor) {
                editor.targetItem = null;
            }

            if (!hoveredItem) {
                dir.clearSelection();

                return;
            }

            var pos = mapToItem(hoveredItem.actionsOverlay, mouse.x, mouse.y);

            if (pos.x > hoveredItem.actionsOverlay.width && pos.y > hoveredItem.actionsOverlay.height) {
                if (gridView.shiftPressed && gridView.currentIndex != -1) {
                    dir.setRangeSelected(gridView.anchorIndex, hoveredItem.index);
                } else {
                    if (!gridView.ctrlPressed && !(mouse.buttons & Qt.RightButton)) {
                        dir.clearSelection();
                    }

                    dir.setSelected(hoveredItem.index);
                }

                gridView.currentIndex = hoveredItem.index;

                if (mouse.buttons & Qt.RightButton) {
                    dir.openContextMenu();
                }
            }
        }

        onClicked: {
            if (mouse.buttons & Qt.RightButton) {
                return;
            }

            if (!hoveredItem || gridView.currentIndex == -1 || gridView.ctrlPressed || gridView.shiftPressed) {
                return;
            }

            var pos = mapToItem(hoveredItem.actionsOverlay, mouse.x, mouse.y);

            if (pos.x > hoveredItem.actionsOverlay.width && pos.y > hoveredItem.actionsOverlay.height) {
                if (systemSettings.singleClick || doubleClickInProgress) {
                    dir.run(gridView.currentIndex);
                } else {
                    doubleClickInProgress = true;
                    doubleClickTimer.interval = systemSettings.doubleClickInterval();
                    doubleClickTimer.start();
                }
            }
        }

        onPositionChanged: {
            var pos = mapToItem(gridView.contentItem, mouse.x, mouse.y);
            var item = gridView.itemAt(pos.x, pos.y);

            if (!item) {
                gridView.hoveredItem = null;
            }
        }

        onHoveredItemChanged: {
            doubleClickInProgress = false;
        }

        Timer {
            id: doubleClickTimer

            onTriggered: {
                doubleClickInProgress = false;
            }
        }

        PlasmaExtras.ScrollArea {
            id: scrollArea

            anchors {
                fill: parent
            }

            GridView {
                id: gridView

                property bool isRootView: false
                property int iconSize: FolderTools.iconSizeFromTheme(plasmoid.configuration.iconSize) // TODO: DPI can change at runtime. Invalidate.
                property Item hoveredItem: null
                property int anchorIndex: 0
                property bool ctrlPressed: false
                property bool shiftPressed: false

                currentIndex: -1

                focus: true

                interactive: true
                keyNavigationWraps: false
                boundsBehavior: Flickable.StopAtBounds

                cellWidth: iconSize + (4 * units.largeSpacing)
                cellHeight: (iconSize + (theme.mSize(theme.defaultFont).height * plasmoid.configuration.textLines) + (2 * units.largeSpacing) + (2 * units.smallSpacing))

                model: dir

                delegate: ItemDelegate {
                    width: gridView.cellWidth
                    height: gridView.cellHeight
                }

                onCurrentIndexChanged: {
                    positionViewAtIndex(currentIndex, GridView.Contain);
                }

                function updateSelection(modifier) {
                    if (modifier & Qt.ShiftModifier) {
                        dir.setRangeSelected(anchorIndex, currentIndex);
                    } else {
                        dir.clearSelection();
                        dir.setSelected(currentIndex);
                    }
                }

                Keys.onReturnPressed: {
                    if (currentIndex != -1) {
                        dir.run(currentIndex);
                    }
                }

                Keys.onMenuPressed: {
                    if (currentIndex != -1) {
                        dir.setSelected(currentIndex);
                        dir.openContextMenu(); // FIXME TODO: Position.
                    }
                }

                Keys.onPressed: {
                    if (event.key == Qt.Key_Control) {
                        ctrlPressed = true;
                    } else if (event.key == Qt.Key_Shift) {
                        shiftPressed = true;

                        if (currentIndex != -1) {
                            anchorIndex = currentIndex;
                        }
                    }
                }

                Keys.onReleased: {
                    if (event.key == Qt.Key_Control) {
                        ctrlPressed = true;
                    } else if (event.key == Qt.Key_Shift) {
                        shiftPressed = false;
                        anchorIndex = 0;
                    }
                }

                Keys.onLeftPressed: {
                    var oldIndex = currentIndex;

                    moveCurrentIndexLeft();

                    if (oldIndex == currentIndex) {
                        return;
                    }

                    updateSelection(event.modifiers);
                }

                Keys.onRightPressed: {
                    var oldIndex = currentIndex;

                    moveCurrentIndexRight();

                    if (oldIndex == currentIndex) {
                        return;
                    }

                    updateSelection(event.modifiers);
                }

                Keys.onUpPressed: {
                    var oldIndex = currentIndex;

                    moveCurrentIndexUp();

                    if (oldIndex == currentIndex) {
                        return;
                    }

                    updateSelection(event.modifiers);
                }

                Keys.onDownPressed: {
                    var oldIndex = currentIndex;

                    moveCurrentIndexDown();

                    if (oldIndex == currentIndex) {
                        return;
                    }

                    updateSelection(event.modifiers);
                }
            }
        }

        Folder.FolderModel {
            id: dir

            usedByContainment: root.isContainment
            sortMode: plasmoid.configuration.sortMode
            sortDesc: plasmoid.configuration.sortDesc
            sortDirsFirst: plasmoid.configuration.sortDirsFirst
            parseDesktopFiles: (url == "desktop:/")
            previews: plasmoid.configuration.previews
            previewPlugins: plasmoid.configuration.previewPlugins
        }

        Folder.ItemViewAdapter {
            id: viewAdapter

            adapterView: gridView
            adapterModel: dir;
            adapterIconSize: gridView.iconSize;
            adapterVisibleArea: Qt.rect(gridView.contentX, gridView.contentY, gridView.width, gridView.height)

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
                    x = pos.x + units.smallSpacing;
                    y = pos.y + units.smallSpacing;
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
                    gridView.focus = true;
                }
            }

            onAccepted: {
                dir.rename(targetItem.index, text);
                targetItem = null;
            }
        }

        Component.onCompleted: {
            dir.requestRename.connect(rename);
        }
    }

}
