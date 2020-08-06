/*
 *    Copyright 2015 Sebastian Kügler <sebas@kde.org>
 *    Copyright 2016 Anthony Fieroni <bvbfan@abv.bg>
 *    Copyright 2018 David Edmundson <davidedmundson@kde.org>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 2.7
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.taskmanager 0.1 as TaskManager

Item {
    id: root

    readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
        || plasmoid.location === PlasmaCore.Types.RightEdge
        || plasmoid.location === PlasmaCore.Types.BottomEdge
        || plasmoid.location === PlasmaCore.Types.LeftEdge)

    Layout.minimumWidth: units.gridUnit
    Layout.minimumHeight: units.gridUnit

    Layout.maximumWidth: inPanel ? units.iconSizeHints.panel : -1
    Layout.maximumHeight: inPanel ? units.iconSizeHints.panel : -1

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    Plasmoid.onActivated: toggleActive()

    property bool active: false
    property var minimizedClients: [] //list of persistentmodelindexes from task manager model of clients minimised by us

    function activate() {
        var clients = []
        for (var i = 0 ; i < tasksModel.count; i++) {
            var idx = tasksModel.makeModelIndex(i);
            if (!tasksModel.data(idx, TaskManager.AbstractTasksModel.IsMinimized)) {
                tasksModel.requestToggleMinimized(idx);
                clients.push(tasksModel.makePersistentModelIndex(i));
            }
        }
        root.minimizedClients = clients;
        root.active = true;
    }

    function deactivate() {
        root.active = false;
        for (var i = 0 ; i < root.minimizedClients.length; i++) {
            var idx = root.minimizedClients[i]
            //client deleted, do nothing
            if (!idx.valid) {
                continue;
            }
            //if the user has restored it already, do nothing
            if (!tasksModel.data(idx, TaskManager.AbstractTasksModel.IsMinimized)) {
                continue;
            }
            tasksModel.requestToggleMinimized(idx);
        }
        root.minimizedClients = [];
    }

    function toggleActive() {
        if (root.active) {
            deactivate();
        } else {
            activate();
        }
    }

    TaskManager.TasksModel {
        id: tasksModel
        sortMode: TaskManager.TasksModel.SortDisabled
        groupMode: TaskManager.TasksModel.GroupDisabled
    }

    Connections {
        target: tasksModel
        enabled: root.active

        onActiveTaskChanged: {
            if (tasksModel.activeTask.valid) { //to suppress changing focus to non windows, such as the desktop
                root.active = false;
                root.minimizedClients = [];
            }
        }
        onVirtualDesktopChanged: deactivate()
        onActivityChanged: deactivate()
    }

    PlasmaCore.FrameSvgItem {
        id: expandedItem
        anchors.fill: parent
        imagePath: "widgets/tabbar"
        prefix: {
            var prefix;
            switch (plasmoid.location) {
                case PlasmaCore.Types.LeftEdge:
                    prefix = "west-active-tab";
                    break;
                case PlasmaCore.Types.TopEdge:
                    prefix = "north-active-tab";
                    break;
                case PlasmaCore.Types.RightEdge:
                    prefix = "east-active-tab";
                    break;
                default:
                    prefix = "south-active-tab";
            }
            if (!hasElementPrefix(prefix)) {
                prefix = "active-tab";
            }
            return prefix;
        }
        opacity: root.active ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: units.shortDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    PlasmaCore.IconItem {
        id:icon
        source: plasmoid.configuration.icon
        active: tooltip.containsMouse
        anchors.fill: parent
    }

    PlasmaCore.ToolTipArea {
        id: tooltip
        anchors.fill: parent
        mainText : i18n("Minimize all Windows")
        subText : i18n("Show the desktop by minimizing all windows")

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            onClicked: root.toggleActive()
        }
        //also activate when dragging an item over the plasmoid so a user can easily drag data to the desktop
        DropArea {
            anchors.fill: parent
            onEntered: activateTimer.start()
            onExited: activateTimer.stop()
            Timer {
                id: activateTimer
                interval: 250 //to match TaskManager
                onTriggered: toggleActive()
            }
        }
    }
}
