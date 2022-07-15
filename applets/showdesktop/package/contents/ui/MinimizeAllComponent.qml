/*
    SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2016 Anthony Fieroni <bvbfan@abv.bg>
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.taskmanager 0.1 as TaskManager

QtObject {
    id: component

    readonly property QtObject tasksModel: TaskManager.TasksModel {
        sortMode: TaskManager.TasksModel.SortDisabled
        groupMode: TaskManager.TasksModel.GroupDisabled
    }

    readonly property Connections activeTaskChangedConnection: Connections {
        target: tasksModel
        enabled: component.active

        function onActiveTaskChanged() {
            if (tasksModel.activeTask.valid) { // to suppress changing focus to non windows, such as the desktop
                component.active = false;
                component.minimizedClients = [];
            }
        }

        function onVirtualDesktopChanged() {
            component.deactivate();
        }

        function onActivityChanged() {
            component.deactivate();
        }
    }

    /**
     * Whether the "minimize all" effect is activated
     */
    property bool active: false

    /**
     * List of persistent model indexes from task manager model of
     * clients minimized by us
     */
    property var minimizedClients: []

    function activate() {
        const clients = [];
        for (let i = 0 ; i < tasksModel.count; i++) {
            const idx = tasksModel.makeModelIndex(i);
            if (!tasksModel.data(idx, TaskManager.AbstractTasksModel.IsHidden)) {
                tasksModel.requestToggleMinimized(idx);
                clients.push(tasksModel.makePersistentModelIndex(i));
            }
        }
        component.minimizedClients = clients;
        component.active = true;
    }

    function deactivate() {
        component.active = false;
        for (let i = 0 ; i < component.minimizedClients.length; i++) {
            const idx = component.minimizedClients[i];
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
        component.minimizedClients = [];
    }

    function toggleActive() {
        if (component.active) {
            deactivate();
        } else {
            activate();
        }
    }
}
