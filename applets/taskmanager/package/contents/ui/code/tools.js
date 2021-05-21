/*
    SPDX-FileCopyrightText: 2012-2016 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

.import org.kde.taskmanager 0.1 as TaskManager
.import org.kde.plasma.core 2.0 as PlasmaCore // Needed by TaskManager

function wheelActivateNextPrevTask(anchor, wheelDelta, eventDelta) {
    // magic number 120 for common "one click"
    // See: http://qt-project.org/doc/qt-5/qml-qtquick-wheelevent.html#angleDelta-prop
    wheelDelta += eventDelta;
    var increment = 0;
    while (wheelDelta >= 120) {
        wheelDelta -= 120;
        increment++;
    }
    while (wheelDelta <= -120) {
        wheelDelta += 120;
        increment--;
    }
    while (increment != 0) {
        activateNextPrevTask(anchor, increment < 0)
        increment += (increment < 0) ? 1 : -1;
    }

    return wheelDelta;
}

function activateNextPrevTask(anchor, next) {
    // FIXME TODO: Unnecessarily convoluted and costly; optimize.

    var taskIndexList = [];
    var activeTaskIndex = tasksModel.activeTask;

    for (var i = 0; i < taskList.children.length - 1; ++i) {
        var task = taskList.children[i];
        var modelIndex = task.modelIndex(i);

        if (task.m.IsLauncher !== true && task.m.IsStartup !== true) {
            if (task.m.IsGroupParent === true) {
                if (task == anchor) { // If the anchor is a group parent, collect only windows within the group.
                    taskIndexList = [];
                }

                for (var j = 0; j < tasksModel.rowCount(modelIndex); ++j) {
                    const childModelIndex = tasksModel.makeModelIndex(i, j);
                    if (!task.m.IsHidden) {
                        taskIndexList.push(childModelIndex);
                    }
                }

                if (task == anchor) { // See above.
                    break;
                }
            } else {
                if (!task.m.IsHidden) {
                    taskIndexList.push(modelIndex);
                }
            }
        }
    }

    if (!taskIndexList.length) {
        return;
    }

    var target = taskIndexList[0];

    for (var i = 0; i < taskIndexList.length; ++i) {
        if (taskIndexList[i] === activeTaskIndex)
        {
            if (next && i < (taskIndexList.length - 1)) {
                target = taskIndexList[i + 1];
            } else if (!next) {
                if (i) {
                    target = taskIndexList[i - 1];
                } else {
                    target = taskIndexList[taskIndexList.length - 1];
                }
            }

            break;
        }
    }

    tasksModel.requestActivate(target);
}

function activateTask(index, model, modifiers, task) {
    if (modifiers & Qt.ShiftModifier) {
        tasksModel.requestNewInstance(index);
    } else if (model.IsGroupParent === true) {

        // Option 1 (default): Cycle through this group's tasks
        // ====================================================
        // If the grouped task does not include the currently active task, bring
        // forward the most recently used task in the group according to the
        // Stacking order.
        // Otherwise cycle through all tasks in the group without paying attention
        // to the stacking order, which otherwise would change with every click
        if (plasmoid.configuration.groupedTaskVisualization === 0) {
            let childTaskList = [];
            let highestStacking = -1;
            let lastUsedTask = undefined;

            // Build list of child tasks and get stacking order data for them
            for (let i = 0; i < tasksModel.rowCount(task.modelIndex(index)); ++i) {
                const childTaskModelIndex = tasksModel.makeModelIndex(task.itemIndex, i);
                childTaskList.push(childTaskModelIndex);
                const stacking = tasksModel.data(childTaskModelIndex, TaskManager.AbstractTasksModel.StackingOrder);
                if (stacking > highestStacking) {
                    highestStacking = stacking;
                    lastUsedTask = childTaskModelIndex;
                }
            }

            // If the active task is from a different app from the group that
            // was clicked on switch to the last-used task from that app.
            if (!childTaskList.some(index => tasksModel.data(index, TaskManager.AbstractTasksModel.IsActive))) {
                tasksModel.requestActivate(lastUsedTask);
            } else {
                // If the active task is already among in the group that was
                // activated, cycle through all tasks according to the order of
                // the immutable model index so the order doesn't change with
                // every click.
                for (let j = 0; j < childTaskList.length; ++j) {
                    const childTask = childTaskList[j];
                        if (tasksModel.data(childTask, TaskManager.AbstractTasksModel.IsActive)) {
                            // Found the current task. Activate the next one
                            let nextTask = j + 1;
                            if (nextTask >= childTaskList.length) {
                                nextTask = 0;
                            }
                            tasksModel.requestActivate(childTaskList[nextTask]);
                            break;
                        }
                }
            }
        }

        // Option 2: show tooltips for all child tasks
        // ===========================================
        // Make sure tooltips are actually enabled though; if not, fall through
        // to the next option.
        else if (plasmoid.configuration.showToolTips
            && plasmoid.configuration.groupedTaskVisualization === 1
        ) {
            task.showToolTip();
        }

        // Option 3: show Present Windows for all child tasks
        // ==================================================
        // Make sure the Present Windows effect is  are actually enabled though;
        // if not, fall through to the next option.
        else if (backend.canPresentWindows
            && (plasmoid.configuration.groupedTaskVisualization === 2
            || plasmoid.configuration.groupedTaskVisualization === 1)
        ) {
            task.hideToolTipTemporarily();
            tasks.presentWindows(model.WinIdList);
        }

        // Option 4: show group dialog/textual list
        // ========================================
        // This is also the final fallback option if Tooltips or Present windows
        // are chosen but not actully available
        else {
            if (groupDialog.visible) {
                task.hideToolTipTemporarily();
                groupDialog.visible = false;
            } else {
                groupDialog.visualParent = task;
                groupDialog.visible = true;
            }
        }
    } else {
        if (model.IsMinimized === true) {
            tasksModel.requestToggleMinimized(index);
            tasksModel.requestActivate(index);
        } else if (model.IsActive === true && plasmoid.configuration.minimizeActiveTaskOnClick) {
            tasksModel.requestToggleMinimized(index);
        } else {
            tasksModel.requestActivate(index);
        }
    }
}

function insertIndexAt(above, x, y) {
    if (above) {
        return above.itemIndex;
    } else {
        var distance = tasks.vertical ? x : y;
        var step = tasks.vertical ? LayoutManager.taskWidth() : LayoutManager.taskHeight();
        var stripe = Math.ceil(distance / step);

        if (stripe === LayoutManager.calculateStripes()) {
            return tasksModel.count - 1;
        } else {
            return stripe * LayoutManager.tasksPerStripe();
        }
    }
}

function publishIconGeometries(taskItems) {
    for (var i = 0; i < taskItems.length - 1; ++i) {
        var task = taskItems[i];

        if (task.IsLauncher !== true && task.m.IsStartup !== true) {
            tasksModel.requestPublishDelegateGeometry(tasksModel.makeModelIndex(task.itemIndex),
                backend.globalRect(task), task);
        }
    }
}

function taskPrefix(prefix) {
    var effectivePrefix;

    switch (plasmoid.location) {
    case PlasmaCore.Types.LeftEdge:
        effectivePrefix = "west-" + prefix;
        break;
    case PlasmaCore.Types.TopEdge:
        effectivePrefix = "north-" + prefix;
        break;
    case PlasmaCore.Types.RightEdge:
        effectivePrefix = "east-" + prefix;
        break;
    default:
        effectivePrefix = "south-" + prefix;
    }
    return [effectivePrefix, prefix];
}

function taskPrefixHovered(prefix) {
    var effectivePrefix = taskPrefix(prefix);

    if ("" !== prefix)
        effectivePrefix = [
            ...taskPrefix(prefix + "-hover"),
            ...taskPrefix("hover"),
            ...effectivePrefix
        ];

    return effectivePrefix;
}
