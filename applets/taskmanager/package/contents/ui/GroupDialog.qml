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

import QtQuick 2.7
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
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
    property int contentWidth: scrollArea.overflowing ? mainItem.width - (units.smallSpacing * 3) : mainItem.width
    property TextMetrics textMetrics: TextMetrics {}
    property alias overflowing: scrollArea.overflowing
    property alias activeTask: focusActiveTaskTimer.targetIndex

    function selectTask(task) {
        if (!task) {
            return;
        }

        task.forceActiveFocus();
        scrollArea.ensureItemVisible(task);
    }

    mainItem: PlasmaExtras.ScrollArea {
        id: scrollArea

        property bool overflowing: (viewport.height < contentItem.height)

        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

        function ensureItemVisible(item) {
            var itemTop = item.y;
            var itemBottom = (item.y + item.height);

            if (itemTop < flickableItem.contentY) {
                flickableItem.contentY = itemTop;
            }

            if ((itemBottom - flickableItem.contentY) > viewport.height) {
                flickableItem.contentY = Math.abs(viewport.height - itemBottom);
            }
        }

        MouseHandler {
            id: mouseHandler

            width: parent.width
            height: (groupRepeater.count * (LayoutManager.verticalMargins()
                + Math.max(theme.mSize(theme.defaultFont).height, units.iconSizes.medium)))

            target: taskList
            handleWheelEvents: !scrollArea.overflowing

            Timer {
                id: focusActiveTaskTimer

                property var targetIndex: null

                interval: 0
                repeat: false

                onTriggered: {
                    // Now we can home in on the previously active task
                    // collected in groupDialog.onVisibleChanged.

                    if (targetIndex != null) {
                        for (var i = 0; i < groupRepeater.count; ++i) {
                            var task = groupRepeater.itemAt(i);

                            if (task.modelIndex() == targetIndex) {
                                selectTask(task);
                                return;
                            }
                        }
                    }
                }
            }

            TaskList {
                id: taskList

                anchors.fill: parent

                add: Transition {
                    // We trigger a null-interval timer in the first add
                    // transition after setting the model so onTriggered
                    // will run after the Flow has positioned items.

                    ScriptAction {
                        script: {
                            if (groupRepeater.aboutToPopulate) {
                                focusActiveTaskTimer.restart();
                                groupRepeater.aboutToPopulate = false;
                            }
                        }
                    }
                }

                Repeater {
                    id: groupRepeater

                    property bool aboutToPopulate: false

                    function currentIndex() {
                        for (var i = 0; i < count; ++i) {
                            if (itemAt(i).activeFocus) {
                                return i;
                            }
                        }

                        return -1;
                    }

                    onCountChanged: updateSize();
                }
            }
        }

        Keys.onUpPressed: {
            var currentIndex = groupRepeater.currentIndex();
            // In doubt focus the last item, so we start at the bottom when user
            // initially presses up.
            if (currentIndex === -1) {
                selectTask(groupRepeater.itemAt(groupRepeater.count - 1));
                return;
            }

            var previousIndex = currentIndex - 1;
            if (previousIndex < 0) {
                previousIndex = groupRepeater.count - 1;
            }

            selectTask(groupRepeater.itemAt(previousIndex));
        }

        Keys.onDownPressed: {
            var currentIndex = groupRepeater.currentIndex();
            // In doubt focus the first item, also wrap around.
            if (currentIndex === -1 || currentIndex + 1 >= groupRepeater.count) {
                selectTask(groupRepeater.itemAt(0));
                return;
            }

            selectTask(groupRepeater.itemAt(currentIndex + 1));
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

            groupRepeater.aboutToPopulate = true;
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

                textMetrics.text = task.labelText;
                var textWidth = textMetrics.boundingRect.width;

                if (textWidth > maxWidth) {
                    maxWidth = textWidth;
                }

                task.labelTextChanged.connect(updateSize);
            }

            maxWidth += LayoutManager.horizontalMargins() + units.iconSizes.medium + 2 * units.smallSpacing;
            maxHeight = groupRepeater.count * (LayoutManager.verticalMargins() + Math.max(theme.mSize(theme.defaultFont).height, units.iconSizes.medium));

            mainItem.height = Math.min(preferredHeight, maxHeight);
            mainItem.width = Math.min(preferredWidth, (tasks.vertical ? Math.max(maxWidth, tasks.width) : Math.min(maxWidth, tasks.width)));
        }
    }
}
