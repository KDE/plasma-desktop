/*
    SPDX-FileCopyrightText: 2012-2016 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.taskmanager 0.1 as TaskManager

import "code/tools.js" as TaskTools

DropArea {
    signal urlsDropped(var urls)

    property Item target
    property Item ignoredItem
    property Item hoveredItem
    property bool isGroupDialog: false
    property bool moved: false

    property alias handleWheelEvents: wheelHandler.handleWheelEvents

    function insertIndexAt(above, x, y) {
        if (above) {
            return above.itemIndex;
        } else if (plasmoid.configuration.maxStripes === 1) {
            return Math.ceil((tasks.vertical ? x + taskList.contentX : y + taskList.contentY) / taskList.totalLength);
        } else {
            const distance = tasks.vertical ? x : y;
            const step = tasks.vertical ? taskGrid.taskWidth : taskGrid.taskHeight;
            const stripe = Math.ceil(distance / step);

            if (stripe === taskGrid.currentStripes) {
                return tasks.tasksModel.count - 1;
            } else {
                return stripe * taskGrid.tasksPerStripe;
            }
        }
    }

    //ignore anything that is neither internal to TaskManager or a URL list
    onEntered: event => {
        if (event.formats.indexOf("text/x-plasmoidservicename") >= 0) {
            event.accepted = false;
        }
    }

    onPositionChanged: event => {
        if (target.animating) {
            return;
        }

        let above;
        if (isGroupDialog) {
            above = target.itemAt(event.x, event.y);
        } else if (plasmoid.configuration.maxStripes === 1) {
            above = target.itemAt(event.x + taskList.contentX, event.y + taskList.contentY);
        } else {
            above = target.childAt(event.x, event.y);
        }

        if (!above) {
            hoveredItem = null;
            activationTimer.stop();

            return;
        }

        // If we're mixing launcher tasks with other tasks and are moving
        // a (small) launcher task across a non-launcher task, don't allow
        // the latter to be the move target twice in a row for a while, as
        // it will naturally be moved underneath the cursor as result of the
        // initial move, due to being far larger than the launcher delegate.
        // TODO: This restriction (minus the timer, which improves things)
        // has been proven out in the EITM fork, but could be improved later
        // by tracking the cursor movement vector and allowing the drag if
        // the movement direction has reversed, establishing user intent to
        // move back.
        if (!plasmoid.configuration.separateLaunchers && tasks.dragSource != null
                && tasks.dragSource.m.IsLauncher && !above.m.IsLauncher
                && above === ignoredItem) {
            return;
        } else {
            ignoredItem = null;
        }

        if (tasksModel.sortMode === TaskManager.TasksModel.SortManual && tasks.dragSource) {
            // Reject drags between different TaskList instances.
            if (tasks.dragSource.parent !== above.parent) {
                return;
            }

            var insertAt = insertIndexAt(above, event.x, event.y);

            if (tasks.dragSource !== above && tasks.dragSource.itemIndex !== insertAt) {
                if (!!tasks.groupDialog) {
                    tasksModel.move(tasks.dragSource.itemIndex, insertAt,
                        tasksModel.makeModelIndex(tasks.groupDialog.visualParent.itemIndex));
                } else {
                    tasksModel.move(tasks.dragSource.itemIndex, insertAt);
                }

                ignoredItem = above;
                ignoreItemTimer.restart();
            }
        } else if (!tasks.dragSource && hoveredItem !== above) {
            hoveredItem = above;
            activationTimer.restart();
        }
    }

    onExited: {
        hoveredItem = null;
        activationTimer.stop();
    }

    onDropped: event => {
        // Reject internal drops.
        if (event.formats.indexOf("application/x-orgkdeplasmataskmanager_taskbuttonitem") >= 0) {
            event.accepted = false;
            return;
        }

        // Reject plasmoid drops.
        if (event.formats.indexOf("text/x-plasmoidservicename") >= 0) {
            event.accepted = false;
            return;
        }

        if (event.hasUrls) {
            urlsDropped(event.urls);
            return;
        }
    }

    Connections {
        target: tasks

        function onDragSourceChanged() {
            if (!dragSource) {
                ignoredItem = null;
                ignoreItemTimer.stop();
            }
        }
    }

    Timer {
        id: ignoreItemTimer

        repeat: false
        interval: 750

        onTriggered: {
            ignoredItem = null;
        }
    }

    Timer {
        id: activationTimer

        interval: 250
        repeat: false

        onTriggered: {
            if (parent.hoveredItem.m.IsGroupParent) {
                TaskTools.createGroupDialog(parent.hoveredItem, tasks);
            } else if (!parent.hoveredItem.m.IsLauncher) {
                tasksModel.requestActivate(parent.hoveredItem.modelIndex());
            }
        }
    }

    WheelHandler {
        id: wheelHandler

        property bool handleWheelEvents: true
        property int wheelDelta: 0;

        enabled: handleWheelEvents && plasmoid.configuration.wheelEnabled

        onWheel: event => {
            wheelDelta = TaskTools.wheelActivateNextPrevTask(null, wheelDelta, event.angleDelta.y, plasmoid.configuration.wheelSkipMinimized, tasks);
        }
    }
}
