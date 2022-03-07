/*
    SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2016 Anthony Fieroni <bvbfan@abv.bg>
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.7
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.taskmanager 0.1 as TaskManager

Item {
    id: root

    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)

    Layout.minimumWidth: PlasmaCore.Units.gridUnit
    Layout.minimumHeight: PlasmaCore.Units.gridUnit

    Layout.maximumWidth: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1
    Layout.maximumHeight: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    Plasmoid.onActivated: toggleActive()

    property bool active: false
    property var minimizedClients: [] //list of persistentmodelindexes from task manager model of clients minimised by us

    function activate() {
        var clients = []
        for (var i = 0 ; i < tasksModel.count; i++) {
            var idx = tasksModel.makeModelIndex(i);
            if (!tasksModel.data(idx, TaskManager.AbstractTasksModel.IsHidden)) {
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
            if (!tasksModel.data(idx, TaskManager.AbstractTasksModel.IsHidden)) {
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

        function onActiveTaskChanged() {
            if (tasksModel.activeTask.valid) { //to suppress changing focus to non windows, such as the desktop
                root.active = false;
                root.minimizedClients = [];
            }
        }
        function onVirtualDesktopChanged() {deactivate()}
        function onActivityChanged() {deactivate()}
    }

    PlasmaCore.FrameSvgItem {
        property var containerMargins: {
            let item = tooltip;
            while (item.parent) {
                item = item.parent;
                if (item.isAppletContainer) {
                    return item.getMargins;
                }
            }
            return undefined;
        }

        anchors {
            fill: parent
            property bool returnAllMargins: true
            // The above makes sure margin is returned even for side margins
            // that would be otherwise turned off.
            bottomMargin: containerMargins ? -containerMargins('bottom', returnAllMargins) : 0;
            topMargin: containerMargins ? -containerMargins('top', returnAllMargins) : 0;
            leftMargin: containerMargins ? -containerMargins('left', returnAllMargins) : 0;
            rightMargin: containerMargins ? -containerMargins('right', returnAllMargins) : 0;
        }
        imagePath: "widgets/tabbar"
        prefix: {
            var prefix;
            switch (Plasmoid.location) {
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
                duration: PlasmaCore.Units.shortDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    PlasmaCore.IconItem {
        id:icon
        source: Plasmoid.configuration.icon
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
