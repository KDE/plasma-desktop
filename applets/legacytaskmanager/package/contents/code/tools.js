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

function wheelActivateNextPrevTask(parentItem, wheelDelta, eventDelta) {
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
        activateNextPrevTask(parentItem, increment < 0)
        increment += (increment < 0) ? 1 : -1;
    }

    return wheelDelta;
}

function activateNextPrevTask(parentItem, next) {
    var taskIdList;

    if (parentItem && parentItem.isGroupParent) {
         taskIdList = backend.tasksModel.taskIdList(visualModel.modelIndex(parentItem.itemIndex));
    } else {
        taskIdList = backend.tasksModel.taskIdList();
    }

    if (!taskIdList.length) {
        return;
    }

    var activeTaskId = backend.tasksModel.activeTaskId();
    var target = taskIdList[0];

    for (var i = 0; i < taskIdList.length; ++i) {
        if (taskIdList[i] == activeTaskId)
        {
            if (next && i < (taskIdList.length - 1)) {
                target = taskIdList[i + 1];
            } else if (!next)
            {
                if (i) {
                    target = taskIdList[i - 1];
                } else {
                    target = taskIdList[taskIdList.length - 1];
                }
            }

            break;
        }
    }

    activateItem(target, false);
}

function insertionIndexAt(sourceIndex, x, y) {
    var above = target.childAt(x, y);

    if (above) {
        var index = above.itemIndex;

        if (index > sourceIndex) {
            ++index;
        }

        return index;
    } else {
        var distance = tasks.vertical ? x : y;
        var step = tasks.vertical ? LayoutManager.taskWidth() : LayoutManager.taskHeight();
        var stripe = Math.ceil(distance / step);

        if (stripe == LayoutManager.calculateStripes()) {
            return -1;
        } else {
            return stripe * LayoutManager.tasksPerStripe();
        }
    }
}

function publishIconGeometries(taskItems) {
    for (var i = 0; i < taskItems.length - 1; ++i) {
        var task = taskItems[i];

        if (task.isGroupParent) {
            var taskIdList = backend.tasksModel.taskIdList(visualModel.modelIndex(task.itemIndex));

            for (var j = 0; j < taskIdList.length; ++j) {
                tasks.itemGeometryChanged(task, taskIdList[j].itemId);
            }
        } else if (!task.isLauncher) {
            tasks.itemGeometryChanged(task, task.itemId);
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
    if (!frame.hasElementPrefix(effectivePrefix)) {
        return prefix;
    }
    return effectivePrefix;

}
