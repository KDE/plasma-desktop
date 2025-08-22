/*
    SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2016 Anthony Fieroni <bvbfan@abv.bg>
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQml 2.15

import org.kde.taskmanager 0.1 as TaskManager

Controller {
    id: controller

    titleActive: i18nc("@action:button shown as tooltip and @action:inmenu", "Restore Windows")
    titleInactive: i18nc("@action:button shown as tooltip and @action:inmenu", "Minimize All Windows")

    descriptionActive: i18nc("@info:tooltip shown as subtitle", "Restores the previously minimized windows")
    descriptionInactive: i18nc("@info:tooltip shown as subtitle", "Shows the Desktop by minimizing all windows")
    
    active: Boolean(activeByActivityDesktop[activityDesktopId])

    readonly property QtObject virtualDesktopInfo: TaskManager.VirtualDesktopInfo {
        id: virtualDesktopInfo
    }
    
    readonly property QtObject activityInfo: TaskManager.ActivityInfo {
        id: activityInfo
    }
    
    readonly property QtObject tasksModel: TaskManager.TasksModel {
        id: tasksModel
        sortMode: TaskManager.TasksModel.SortLastActivated
        groupMode: TaskManager.TasksModel.GroupDisabled
    }

    readonly property string activityDesktopId: activityInfo.currentActivity + "_" + virtualDesktopInfo.currentDesktop
    
    readonly property var activeByActivityDesktop: ({})
    
    readonly property Connections activeTaskChangedConnection: Connections {
        target: tasksModel
        enabled: controller.active

        function onActiveTaskChanged() {
            
            /** QML bindings are not updated yet if the activity or desktop was 
             * changed; we need to take the info from the TaskManager instead
             * of reading activityDesktopId
             */
            var aDId = tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.Activities) + "_" +
                tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.VirtualDesktops)
            if (tasksModel.activeTask.valid) { // to suppress changing focus to non windows, such as the desktop
                activeByActivityDesktop[aDId] = false;
                activeByActivityDesktopChanged();
                
                controller.minimizedClients[aDId] = [];
            }
        }
    }

    /**
     * List of persistent model indexes from task manager model of
     * clients minimized by us
     */
    property var minimizedClients: ({})

    function activate() {
        const clients = [];
        for (let i = tasksModel.count - 1; i >= 0; i--) {
            const idx = tasksModel.makeModelIndex(i);
            const activities = tasksModel.data(idx, TaskManager.AbstractTasksModel.Activities);
            const desktops = tasksModel.data(idx, TaskManager.AbstractTasksModel.VirtualDesktops);

            // When activities.length === 0 means on all activities. Same thing for desktops.length === 0 on wayland, wwhile on X11 [-1] means on all desktops
            if (!tasksModel.data(idx, TaskManager.AbstractTasksModel.IsHidden) &&
                (activities.length === 0 || activities.includes(activityInfo.currentActivity)) &&
                (desktops.length === 0 || desktops[0] === -1 || desktops.includes(virtualDesktopInfo.currentDesktop))
            ) {
                tasksModel.requestToggleMinimized(idx);
                clients.push(tasksModel.makePersistentModelIndex(i));
            }
        }
        minimizedClients[activityDesktopId] = clients;
        activeByActivityDesktop[activityDesktopId] = true;
        activeByActivityDesktopChanged()
    }

    function deactivate() {
        activeByActivityDesktop[activityDesktopId] = false;
        activeByActivityDesktopChanged()
        for (let i = 0; i < minimizedClients[activityDesktopId].length; i++) {
            const idx = minimizedClients[activityDesktopId][i];
            // client deleted, do nothing
            if (!idx.valid) {
                continue;
            }
            // if the user has restored it already, do nothing
            if (!tasksModel.data(idx, TaskManager.AbstractTasksModel.IsHidden)) {
                continue;
            }
            tasksModel.requestToggleMinimized(idx);
        }
        minimizedClients[activityDesktopId] = [];
    }

    // override
    function toggle() {
        if (active) {
            deactivate();
        } else {
            activate();
        }
    }
}
