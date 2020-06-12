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

import QtQuick 2.0

import org.kde.draganddrop 2.0

import org.kde.taskmanager 0.1 as TaskManager

import "code/tools.js" as TaskTools

Item {
    signal urlsDropped(var urls)

    property Item target
    property Item ignoredItem
    property bool moved: false

    property alias hoveredItem: dropHandler.hoveredItem
    property alias handleWheelEvents: wheelHandler.active

    Timer {
        id: ignoreItemTimer

        repeat: false
        interval: 750

        onTriggered: {
            ignoredItem = null;
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

    DropArea {
        id: dropHandler

        anchors.fill: parent

        preventStealing: true;

        property Item hoveredItem

        //ignore anything that is neither internal to TaskManager or a URL list
        onDragEnter: {
            if (event.mimeData.formats.indexOf("text/x-plasmoidservicename") >= 0) {
                event.ignore();
            }
        }

        onDragMove: {
            if (target.animating) {
                return;
            }

            var above = target.childAt(event.x, event.y);

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
                 && tasks.dragSource.m.IsLauncher === true && above.m.IsLauncher !== true
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

                var insertAt = TaskTools.insertIndexAt(above, event.x, event.y);

                if (tasks.dragSource !== above && tasks.dragSource.itemIndex !== insertAt) {
                    if (groupDialog.visible && groupDialog.visualParent) {
                        tasksModel.move(tasks.dragSource.itemIndex, insertAt,
                            tasksModel.makeModelIndex(groupDialog.visualParent.itemIndex));
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

        onDragLeave: {
            hoveredItem = null;
            activationTimer.stop();
        }

        onDrop: {
            // Reject internal drops.
            if (event.mimeData.formats.indexOf("application/x-orgkdeplasmataskmanager_taskbuttonitem") >= 0) {
                event.ignore();
                return;
            }

            // Reject plasmoid drops.
            if (event.mimeData.formats.indexOf("text/x-plasmoidservicename") >= 0) {
                event.ignore();
                return;
            }

            if (event.mimeData.hasUrls) {
                parent.urlsDropped(event.mimeData.urls);
                return;
            }
        }

        Timer {
            id: activationTimer

            interval: 250
            repeat: false

            onTriggered: {
                if (parent.hoveredItem.m.IsGroupParent === true) {
                    groupDialog.visualParent = parent.hoveredItem;
                    groupDialog.visible = true;
                } else if (parent.hoveredItem.m.IsLauncher !== true) {
                    tasksModel.requestActivate(parent.hoveredItem.modelIndex());
                }
            }
        }
    }

    MouseArea {
        id: wheelHandler

        anchors.fill: parent

        enabled: active && plasmoid.configuration.wheelEnabled

        property bool active: true
        property int wheelDelta: 0;


        onWheel: {
            if (!active) {
                wheel.accepted = false;
                return;
            }

            wheelDelta = TaskTools.wheelActivateNextPrevTask(null, wheelDelta, wheel.angleDelta.y);
        }
    }
}
