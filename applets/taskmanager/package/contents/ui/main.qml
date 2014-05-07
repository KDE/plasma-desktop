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

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.private.taskmanager 0.1 as TaskManager

import "../code/layout.js" as LayoutManager
import "../code/tools.js" as TaskTools

Item {
    id: tasks

    anchors.fill: parent

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    Layout.fillWidth: true
    Layout.fillHeight:true
    Layout.minimumWidth: tasks.vertical ? 0 : LayoutManager.preferredMinWidth()
    Layout.minimumHeight: !tasks.vertical ? 0 : LayoutManager.preferredMinHeight()

//BEGIN TODO: this is not precise enough: launchers are smaller than full tasks
    Layout.preferredWidth: tasks.vertical ? units.gridUnit * 10 : Math.min(taskList.columns, taskList.children.length) * units.gridUnit * 10;
    Layout.preferredHeight: tasks.vertical ? Math.min(taskList.rows, taskList.children.length) * units.gridUnit * 2 : units.gridUnit * 2;
//END TODO

    property Item dragSource: null

    signal activateItem(int id, bool toggle)
    signal activateWindow(int winId)
    signal itemContextMenu(Item item, QtObject configAction)
    signal itemHovered(int id, bool hovered)
    signal itemMove(int id, int newIndex)
    signal itemGeometryChanged(Item item, int id)
    signal presentWindows(int groupParentId)

    onWidthChanged: {
        taskList.width = LayoutManager.layoutWidth();

        if (plasmoid.configuration.forceStripes) {
            taskList.height = LayoutManager.layoutHeight();
        }
    }

    onHeightChanged: {
        if (plasmoid.configuration.forceStripes) {
            taskList.width = LayoutManager.layoutWidth();
        }

        taskList.height = LayoutManager.layoutHeight();
    }

    TaskManager.Backend {
        id: backend

        taskManagerItem: tasks
        highlightWindows: plasmoid.configuration.highlightWindows

        groupingStrategy: plasmoid.configuration.groupingStrategy
        sortingStrategy: plasmoid.configuration.sortingStrategy

        onLaunchersChanged: plasmoid.configuration.launchers = launchers

        Component.onCompleted: {
            launchers = plasmoid.configuration.launchers;
        }
    }

    Connections {
        target: plasmoid.configuration
        onLaunchersChanged: backend.launchers = plasmoid.configuration.launchers
    }

    Binding {
        target: backend.groupManager
        property: "screen"
        value: plasmoid.screen
    }

    Binding {
        target: backend.groupManager
        property: "onlyGroupWhenFull"
        value: plasmoid.configuration.onlyGroupWhenFull
    }

    Binding {
        target: backend.groupManager
        property: "fullLimit"
        value: LayoutManager.optimumCapacity(width, height) + 1
    }

    Binding {
        target: backend.groupManager
        property: "showOnlyCurrentScreen"
        value: plasmoid.configuration.showOnlyCurrentScreen
    }

    Binding {
        target: backend.groupManager
        property: "showOnlyCurrentDesktop"
        value: plasmoid.configuration.showOnlyCurrentDesktop
    }

    Binding {
        target: backend.groupManager
        property: "showOnlyCurrentActivity"
        value: plasmoid.configuration.showOnlyCurrentActivity
    }

    Binding {
        target: backend.groupManager
        property: "showOnlyMinimized"
        value: plasmoid.configuration.showOnlyMinimized
    }

    TaskManager.DragHelper { id: dragHelper }

    PlasmaCore.FrameSvgItem {
        id: taskFrame

        visible: false;

        imagePath: "widgets/tasks";
        prefix: "normal"
    }

    PlasmaCore.Svg {
        id: taskSvg

        imagePath: "widgets/tasks"
    }

    MouseHandler {
        id: mouseHandler

        anchors.fill: parent

        target: taskList
    }

    VisualDataModel {
        id: visualModel

        model: backend.tasksModel
        delegate: Task {}
    }

    ToolTipDelegate {
        id: toolTipDelegate
    }

    TaskList {
        id: taskList

        anchors {
            left: parent.left
            top: parent.top
        }

        onWidthChanged: LayoutManager.layout(taskRepeater)
        onHeightChanged: LayoutManager.layout(taskRepeater)

        flow: tasks.vertical ? Flow.TopToBottom : Flow.LeftToRight

        onAnimatingChanged: {
            if (!animating) {
                TaskTools.publishIconGeometries(children);
            }
        }

        Repeater {
            id: taskRepeater

            model: visualModel

            onCountChanged: {
                taskList.width = LayoutManager.layoutWidth();
                taskList.height = LayoutManager.layoutHeight();
                LayoutManager.layout(taskRepeater);
            }

            function modelWasReset() {
                LayoutManager.layout(taskRepeater);
            }

            Component.onCompleted: {
                backend.tasksModel.modelReset.connect(modelWasReset);
            }
        }
    }

    GroupDialog { id: groupDialog }

    function hasLauncher(url) {
        return backend.groupManager.launcherExists(url);
    }

    function addLauncher(url) {
        backend.groupManager.addLauncher(url);
    }

    function updateStatus(demandsAttention) {
        if (demandsAttention) {
            plasmoid.status = PlasmaCore.Types.NeedsAttentionStatus;
        } else if (!backend.anyTaskNeedsAttention) {
            plasmoid.status = PlasmaCore.Types.PassiveStatus;
        }
    }

    function resetDragSource() {
        dragSource = null;
    }

    Component.onCompleted: {
        tasks.activateItem.connect(backend.activateItem);
        tasks.activateWindow.connect(backend.activateWindow);
        tasks.itemContextMenu.connect(backend.itemContextMenu);
        tasks.itemHovered.connect(backend.itemHovered);
        tasks.itemMove.connect(backend.itemMove);
        tasks.itemGeometryChanged.connect(backend.itemGeometryChanged);
        tasks.presentWindows.connect(backend.presentWindows);
        dragHelper.dropped.connect(resetDragSource);
    }
}
