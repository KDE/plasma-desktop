/***************************************************************************
 *   Copyright (C) 2012-2013 by Eike Hein <hein@kde.org>                   *
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

import QtQuick 2.0
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.draganddrop 2.0

import "../code/layout.js" as LayoutManager

PlasmaCore.Dialog {
    id: groupDialog
    visible: false

    type: PlasmaCore.Dialog.PopupMenu
    flags: Qt.WindowStaysOnTopHint
    hideOnWindowDeactivate: true
    location: plasmoid.location

    property int preferredWidth: Screen.width / (3 * Screen.devicePixelRatio)
    property int preferredHeight: Screen.height / (2 * Screen.devicePixelRatio)

    mainItem: Item {
        MouseHandler {
            id: mouseHandler

            anchors.fill: parent

            target: taskList
        }

        TaskList {
            id: taskList

            anchors.fill: parent

            Repeater {
                id: groupRepeater

                function currentIndex() {
                    for (var i = 0; i < count; ++i) {
                        if (itemAt(i).activeFocus) {
                            return i;
                        }
                    }

                    return -1;
                }

                onCountChanged: updateSize()
            }
        }

        Keys.onUpPressed: {
            var currentIndex = groupRepeater.currentIndex();
            // In doubt focus the first item
            if (currentIndex === -1) {
                groupRepeater.itemAt(0).forceActiveFocus();
                return;
            }

            var previousIndex = currentIndex - 1;
            if (previousIndex < 0) {
                previousIndex = groupRepeater.count - 1;
            }

            groupRepeater.itemAt(previousIndex).forceActiveFocus()
        }

        Keys.onDownPressed: {
            var currentIndex = groupRepeater.currentIndex();
            // In doubt focus the first item, also wrap around.
            if (currentIndex === -1 || currentIndex + 1 >= groupRepeater.count) {
                groupRepeater.itemAt(0).forceActiveFocus();
                return;
            }

            groupRepeater.itemAt(currentIndex + 1).forceActiveFocus();
        }

        Keys.onEscapePressed: groupDialog.visible = false;
    }

    data: [
        VisualDataModel {
            id: groupFilter

            delegate: Task {
                visible: true
                inPopup: true
            }
        }
    ]

    onVisualParentChanged: {
        if (!visualParent) {
            visible = false;
        }
    }

    onVisibleChanged: {
        if (visible && visualParent) {
            groupFilter.model = tasksModel;
            groupFilter.rootIndex = groupFilter.modelIndex(visualParent.itemIndex);
            groupRepeater.model = groupFilter;

            mainItem.forceActiveFocus();
        } else {
            visualParent = null;
            groupRepeater.model = undefined;
            groupFilter.model = undefined;
            groupFilter.rootIndex = undefined;
        }
    }

    function updateSize() {
        if (!visible || !visualParent) {
            return;
        }

        if (!groupRepeater.count) {
            visible = false;
        } else {
            var task;
            var maxWidth = 0;
            var maxHeight = 0;

            backend.cancelHighlightWindows();

            for (var i = 0; i < taskList.children.length - 1; ++i) {
                task = taskList.children[i];

                if (task.textWidth > maxWidth) {
                    maxWidth = task.textWidth;
                }

                task.textWidthChanged.connect(updateSize);
            }

            maxWidth += LayoutManager.horizontalMargins() + units.iconSizes.medium + 2 * units.smallSpacing;
            maxHeight = groupRepeater.count * (LayoutManager.verticalMargins() + Math.max(theme.mSize(theme.defaultFont).height, units.iconSizes.medium));

            mainItem.height = Math.min(preferredHeight, maxHeight);
            mainItem.width = Math.min(preferredWidth, (tasks.vertical ? Math.max(maxWidth, tasks.width) : Math.min(maxWidth, tasks.width)));
        }
    }
}
