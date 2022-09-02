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

    titleActive: i18nc("@action:button", "Restore All Minimized Windows")
    titleInactive: i18nc("@action:button", "Minimize All Windows")

    descriptionActive: i18nc("@info:tooltip", "Restores the previously minimized windows")
    descriptionInactive: i18nc("@info:tooltip", "Shows the Desktop by minimizing all windows")

    readonly property QtObject tasksModel: TaskManager.TasksModel {
        id: tasksModel
        sortMode: TaskManager.TasksModel.SortDisabled
        groupMode: TaskManager.TasksModel.GroupDisabled
    }

    readonly property Connections activeTaskChangedConnection: Connections {
        target: tasksModel
        enabled: controller.active

        function onActiveTaskChanged() {
            if (tasksModel.activeTask.valid) { // to suppress changing focus to non windows, such as the desktop
                controller.active = false;
                controller.minimizedClients = [];
            }
        }

        function onVirtualDesktopChanged() {
            controller.deactivate();
        }

        function onActivityChanged() {
            controller.deactivate();
        }
    }

    /**
     * List of persistent model indexes from task manager model of
     * clients minimized by us
     */
    property var minimizedClients: []

    function activate() {
        const clients = [];
        for (let i = 0; i < tasksModel.count; i++) {
            const idx = tasksModel.makeModelIndex(i);
            if (!tasksModel.data(idx, TaskManager.AbstractTasksModel.IsHidden)) {
                tasksModel.requestToggleMinimized(idx);
                clients.push(tasksModel.makePersistentModelIndex(i));
            }
        }
        minimizedClients = clients;
        active = true;
    }

    function deactivate() {
        active = false;
        for (let i = 0; i < minimizedClients.length; i++) {
            const idx = minimizedClients[i];
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
        minimizedClients = [];
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
