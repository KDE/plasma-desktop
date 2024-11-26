/*
    SPDX-FileCopyrightText: 2012-2016 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

.pragma library

.import org.kde.taskmanager as TaskManager
.import org.kde.plasma.core as PlasmaCore // Needed by TaskManager

// Can't be `let`, or else QML counterpart won't be able to assign to it.
var taskManagerInstanceCount = 0;

function activateNextPrevTask(anchor, next, wheelSkipMinimized, tasks) {
    // FIXME TODO: Unnecessarily convoluted and costly; optimize.

    let taskIndexList = [];
    const activeTaskIndex = tasks.tasksModel.activeTask;

    for (let i = 0; i < tasks.taskList.children.length - 1; ++i) {
        const task = tasks.taskList.children[i];
        const modelIndex = task.modelIndex(i);

        if (!task.model.IsLauncher && !task.model.IsStartup) {
            if (task.model.IsGroupParent) {
                if (task === anchor) { // If the anchor is a group parent, collect only windows within the group.
                    taskIndexList = [];
                }

                for (let j = 0; j < tasks.tasksModel.rowCount(modelIndex); ++j) {
                    const childModelIndex = tasks.tasksModel.makeModelIndex(i, j);
                    const childHidden = tasks.tasksModel.data(childModelIndex, TaskManager.AbstractTasksModel.IsHidden);
                    if (!wheelSkipMinimized || !childHidden) {
                        taskIndexList.push(childModelIndex);
                    }
                }

                if (task === anchor) { // See above.
                    break;
                }
            } else {
                if (!wheelSkipMinimized || !task.model.IsHidden) {
                    taskIndexList.push(modelIndex);
                }
            }
        }
    }

    if (!taskIndexList.length) {
        return;
    }

    let target = taskIndexList[0];

    for (let i = 0; i < taskIndexList.length; ++i) {
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

    tasks.tasksModel.requestActivate(target);
}

function activateTask(index, model, modifiers, task, plasmoid, tasks, windowViewAvailable) {
    if (modifiers & Qt.ShiftModifier) {
        tasks.tasksModel.requestNewInstance(index);
        return;
    }
    // Publish delegate geometry again if there are more than one task manager instance
    if (taskManagerInstanceCount >= 2) {
        tasks.tasksModel.requestPublishDelegateGeometry(task.modelIndex(), tasks.backend.globalRect(task), task);
    }

    if (model.IsGroupParent) {
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
            for (let i = 0; i < tasks.tasksModel.rowCount(task.modelIndex(index)); ++i) {
                const childTaskModelIndex = tasks.tasksModel.makeModelIndex(task.index, i);
                childTaskList.push(childTaskModelIndex);
                const stacking = tasks.tasksModel.data(childTaskModelIndex, TaskManager.AbstractTasksModel.StackingOrder);
                if (stacking > highestStacking) {
                    highestStacking = stacking;
                    lastUsedTask = childTaskModelIndex;
                }
            }

            // If the active task is from a different app from the group that
            // was clicked on switch to the last-used task from that app.
            if (!childTaskList.some(index => tasks.tasksModel.data(index, TaskManager.AbstractTasksModel.IsActive))) {
                tasks.tasksModel.requestActivate(lastUsedTask);
            } else {
                // If the active task is already among in the group that was
                // activated, cycle through all tasks according to the order of
                // the immutable model index so the order doesn't change with
                // every click.
                for (let j = 0; j < childTaskList.length; ++j) {
                    const childTask = childTaskList[j];
                        if (tasks.tasksModel.data(childTask, TaskManager.AbstractTasksModel.IsActive)) {
                            // Found the current task. Activate the next one
                            let nextTask = j + 1;
                            if (nextTask >= childTaskList.length) {
                                nextTask = 0;
                            }
                            tasks.tasksModel.requestActivate(childTaskList[nextTask]);
                            break;
                        }
                }
            }
        }

        // Option 2: show tooltips for all child tasks
        // ===========================================
        else if (plasmoid.configuration.groupedTaskVisualization === 1) {
            if (tasks.toolTipOpenedByClick) {
                task.hideImmediately();
            } else {
                tasks.toolTipOpenedByClick = task;
                task.updateMainItemBindings(); // BUG 452187
                task.showToolTip();
            }
        }

        // Option 3: show Window View for all child tasks
        // ==================================================
        // Make sure the Window View effect is  are actually enabled though;
        // if not, fall through to the next option.
        else if (plasmoid.configuration.groupedTaskVisualization === 2 && windowViewAvailable) {
            task.hideToolTip();
            tasks.activateWindowView(model.WinIdList);
        }

        // Option 4: show group dialog/textual list
        // ========================================
        // This is also the final fallback option if Window View
        // is chosen but not actually available
        else {
            if (tasks.groupDialog) {
                task.hideToolTip();
                tasks.groupDialog.visible = false;
            } else {
                createGroupDialog(task, tasks);
            }
        }
    } else {
        if (model.IsMinimized) {
            tasks.tasksModel.requestToggleMinimized(index);
            tasks.tasksModel.requestActivate(index);
        } else if (model.IsActive && plasmoid.configuration.minimizeActiveTaskOnClick) {
            tasks.tasksModel.requestToggleMinimized(index);
        } else {
            tasks.tasksModel.requestActivate(index);
        }
    }
}

function taskPrefix(prefix, location) {
    let effectivePrefix;

    switch (location) {
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

function taskPrefixHovered(prefix, location) {
    return [
        ...taskPrefix((prefix || "launcher") + "-hover", location),
        ...prefix ? taskPrefix("hover", location) : [],
        ...taskPrefix(prefix, location),
    ];
}

function createGroupDialog(visualParent, tasks) {
    if (!visualParent) {
        return;
    }

    if (tasks.groupDialog) {
        tasks.groupDialog.visualParent = visualParent;
        return;
    }

    tasks.groupDialog = tasks.groupDialogComponent.createObject(tasks, { visualParent });
}
