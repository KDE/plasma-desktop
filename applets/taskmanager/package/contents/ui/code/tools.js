/***************************************************************************
 *   Copyright (C) 2012-2016 by Eike Hein <hein@kde.org>                   *
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
                    taskIndexList.push(tasksModel.makeModelIndex(i, j));
                }

                if (task == anchor) { // See above.
                    break;
                }
            } else {
                taskIndexList.push(modelIndex);
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
        if ((iconsOnly || modifiers == Qt.ControlModifier) && backend.canPresentWindows()) {
            task.toolTipAreaItem.hideToolTip();
            tasks.presentWindows(model.WinIdList);
        } else if (groupDialog.visible) {
            groupDialog.visible = false;
        } else {
            groupDialog.visualParent = task;
            groupDialog.visible = true;
        }
    } else {
        if (model.IsMinimized === true) {
            tasksModel.requestToggleMinimized(index);
            tasksModel.requestActivate(index);
        } else if (model.IsActive === true) {
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
