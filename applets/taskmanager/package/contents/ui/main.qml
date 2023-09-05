/*
    SPDX-FileCopyrightText: 2012-2016 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQml 2.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.plasma5support 2.0 as P5Support
import org.kde.kirigami 2.20 as Kirigami

import org.kde.plasma.workspace.trianglemousefilter 1.0

import org.kde.taskmanager 0.1 as TaskManager
import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

import "code/layout.js" as LayoutManager
import "code/tools.js" as TaskTools

PlasmoidItem {
    id: tasks

    // For making a bottom to top layout since qml flow can't do that.
    // We just hang the task manager upside down to achieve that.
    // This mirrors the tasks as well, so we just rotate them again to fix that (see Task.qml).
    rotation: Plasmoid.configuration.reverseMode && Plasmoid.formFactor === PlasmaCore.Types.Vertical ? 180 : 0

    readonly property bool shouldShirnkToZero: !LayoutManager.logicalTaskCount()
    property bool vertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical
    property bool iconsOnly: Plasmoid.pluginName === "org.kde.plasma.icontasks"

    property var toolTipOpenedByClick: null

    property QtObject contextMenuComponent: Qt.createComponent("ContextMenu.qml")
    property QtObject pulseAudioComponent: Qt.createComponent("PulseAudio.qml")

    property var toolTipAreaItem: null

    property bool needLayoutRefresh: false;
    property variant taskClosedWithMouseMiddleButton: []
    property alias taskList: taskList

    preferredRepresentation: fullRepresentation

    Plasmoid.constraintHints: PlasmaCore.Types.CanFillArea

    Plasmoid.onUserConfiguringChanged: {
        if (Plasmoid.userConfiguring && !!tasks.groupDialog) {
            tasks.groupDialog.visible = false;
        }
    }

    Layout.fillWidth: tasks.vertical ? true : Plasmoid.configuration.fill
    Layout.fillHeight: !tasks.vertical ? true : Plasmoid.configuration.fill
    Layout.minimumWidth: {
        if (shouldShirnkToZero) {
            return Kirigami.Units.gridUnit; // For edit mode
        }
        return tasks.vertical ? 0 : LayoutManager.preferredMinWidth();
    }
    Layout.minimumHeight: {
        if (shouldShirnkToZero) {
            return Kirigami.Units.gridUnit; // For edit mode
        }
        return !tasks.vertical ? 0 : LayoutManager.preferredMinHeight();
    }

//BEGIN TODO: this is not precise enough: launchers are smaller than full tasks
    Layout.preferredWidth: {
        if (shouldShirnkToZero) {
            return 0.01;
        }
        if (tasks.vertical) {
            return Kirigami.Units.gridUnit * 10;
        }
        return (LayoutManager.logicalTaskCount() * LayoutManager.preferredMaxWidth()) / LayoutManager.calculateStripes();
    }
    Layout.preferredHeight: {
        if (shouldShirnkToZero) {
            return 0.01;
        }
        if (tasks.vertical) {
            return (LayoutManager.logicalTaskCount() * LayoutManager.preferredMaxHeight()) / LayoutManager.calculateStripes();
        }
        return Kirigami.Units.gridUnit * 2;
    }
//END TODO

    property Item dragSource: null

    signal requestLayout
    signal windowsHovered(variant winIds, bool hovered)
    signal activateWindowView(variant winIds)

    onDragSourceChanged: {
        if (dragSource == null) {
            tasksModel.syncLaunchers();
        }
    }

    function publishIconGeometries(taskItems) {
        if (TaskTools.taskManagerInstanceCount >= 2) {
            return;
        }
        for (var i = 0; i < taskItems.length - 1; ++i) {
            var task = taskItems[i];

            if (!task.m.IsLauncher && !task.m.IsStartup) {
                tasks.tasksModel.requestPublishDelegateGeometry(tasks.tasksModel.makeModelIndex(task.itemIndex),
                    backend.globalRect(task), task);
            }
        }
    }

    property TaskManager.TasksModel tasksModel: TaskManager.TasksModel {
        id: tasksModel

        readonly property int logicalLauncherCount: {
            if (Plasmoid.configuration.separateLaunchers) {
                return launcherCount;
            }

            var startupsWithLaunchers = 0;

            for (var i = 0; i < taskRepeater.count; ++i) {
                var item = taskRepeater.itemAt(i);

                if (item && item.m.IsStartup && item.m.HasLauncher) {
                    ++startupsWithLaunchers;
                }
            }

            return launcherCount + startupsWithLaunchers;
        }

        virtualDesktop: virtualDesktopInfo.currentDesktop
        screenGeometry: Plasmoid.containment.screenGeometry
        activity: activityInfo.currentActivity

        filterByVirtualDesktop: Plasmoid.configuration.showOnlyCurrentDesktop
        filterByScreen: Plasmoid.configuration.showOnlyCurrentScreen
        filterByActivity: Plasmoid.configuration.showOnlyCurrentActivity
        filterNotMinimized: Plasmoid.configuration.showOnlyMinimized

        sortMode: sortModeEnumValue(Plasmoid.configuration.sortingStrategy)
        launchInPlace: iconsOnly && Plasmoid.configuration.sortingStrategy === 1
        separateLaunchers: {
            if (!iconsOnly && !Plasmoid.configuration.separateLaunchers
                && Plasmoid.configuration.sortingStrategy === 1) {
                return false;
            }

            return true;
        }

        groupMode: groupModeEnumValue(Plasmoid.configuration.groupingStrategy)
        groupInline: !Plasmoid.configuration.groupPopups && !iconsOnly
        groupingWindowTasksThreshold: (Plasmoid.configuration.onlyGroupWhenFull && !iconsOnly
            ? LayoutManager.optimumCapacity(width, height) + 1 : -1)

        onLauncherListChanged: {
            layoutTimer.restart();
            Plasmoid.configuration.launchers = launcherList;
        }

        onGroupingAppIdBlacklistChanged: {
            Plasmoid.configuration.groupingAppIdBlacklist = groupingAppIdBlacklist;
        }

        onGroupingLauncherUrlBlacklistChanged: {
            Plasmoid.configuration.groupingLauncherUrlBlacklist = groupingLauncherUrlBlacklist;
        }

        function sortModeEnumValue(index) {
            switch (index) {
                case 0:
                    return TaskManager.TasksModel.SortDisabled;
                case 1:
                    return TaskManager.TasksModel.SortManual;
                case 2:
                    return TaskManager.TasksModel.SortAlpha;
                case 3:
                    return TaskManager.TasksModel.SortVirtualDesktop;
                case 4:
                    return TaskManager.TasksModel.SortActivity;
                default:
                    return TaskManager.TasksModel.SortDisabled;
            }
        }

        function groupModeEnumValue(index) {
            switch (index) {
                case 0:
                    return TaskManager.TasksModel.GroupDisabled;
                case 1:
                    return TaskManager.TasksModel.GroupApplications;
            }
        }

        Component.onCompleted: {
            launcherList = Plasmoid.configuration.launchers;
            groupingAppIdBlacklist = Plasmoid.configuration.groupingAppIdBlacklist;
            groupingLauncherUrlBlacklist = Plasmoid.configuration.groupingLauncherUrlBlacklist;

            // Only hook up view only after the above churn is done.
            taskRepeater.model = tasksModel;
        }
    }

    property TaskManagerApplet.Backend backend: TaskManagerApplet.Backend {
        taskManagerItem: tasks
        highlightWindows: Plasmoid.configuration.highlightWindows

        onAddLauncher: {
            tasks.addLauncher(url);
        }

        onWindowViewAvailableChanged: TaskTools.windowViewAvailable = windowViewAvailable;

        Component.onCompleted: TaskTools.windowViewAvailable = windowViewAvailable;
    }

    property Component taskInitComponent: Component {
        Timer {
            id: timer

            interval: Kirigami.Units.longDuration
            running: true

            onTriggered: {
                tasksModel.requestPublishDelegateGeometry(parent.modelIndex(), backend.globalRect(parent), parent);
                timer.destroy();
            }
        }
    }

    MouseArea {
        anchors.fill: parent

        hoverEnabled: true
        onExited: {
            if (needLayoutRefresh) {
                LayoutManager.layout(taskRepeater)
                needLayoutRefresh = false;
            }
        }

        TaskManager.VirtualDesktopInfo {
            id: virtualDesktopInfo
        }

        TaskManager.ActivityInfo {
            id: activityInfo
            readonly property string nullUuid: "00000000-0000-0000-0000-000000000000"
        }

        P5Support.DataSource {
            id: mpris2Source
            engine: "mpris2"
            connectedSources: sources
            onSourceAdded: source => connectSource(source);
            onSourceRemoved: source => disconnectSource(source);
            function sourceNameForLauncherUrl(launcherUrl, pid) {
                if (!launcherUrl || launcherUrl === "") {
                    return "";
                }

                // MPRIS spec explicitly mentions that "DesktopEntry" is with .desktop extension trimmed
                // Moreover, remove URL parameters, like wmClass (part after the question mark)
                var desktopFileName = launcherUrl.toString().split('/').pop().split('?')[0].replace(".desktop", "")
                if (desktopFileName.indexOf("applications:") === 0) {
                    desktopFileName = desktopFileName.substr(13)
                }

                let fallbackSource = "";

                for (var i = 0, length = connectedSources.length; i < length; ++i) {
                    var source = connectedSources[i];
                    // we intend to connect directly, otherwise the multiplexer steals the connection away
                    if (source === "@multiplex") {
                        continue;
                    }

                    var sourceData = data[source];
                    if (!sourceData) {
                        continue;
                    }

                    /**
                    * If the task is in a group, we can't use desktopFileName to match the task.
                    * but in case PID match fails, use the match result from desktopFileName.
                    */
                    if (pid && sourceData.InstancePid === pid) {
                        return source;
                    }
                    if (sourceData.DesktopEntry === desktopFileName) {
                        fallbackSource = source;
                    }

                    var metadata = sourceData.Metadata;
                    if (metadata) {
                        var kdePid = metadata["kde:pid"];
                        if (kdePid && pid === kdePid) {
                            return source;
                        }
                    }
                }

                // If PID match fails, return fallbackSource.
                return fallbackSource;
            }

            function startOperation(source, op) {
                var service = serviceForSource(source)
                var operation = service.operationDescription(op)
                return service.startOperationCall(operation)
            }

            function goPrevious(source) {
                startOperation(source, "Previous");
            }
            function goNext(source) {
                startOperation(source, "Next");
            }
            function play(source) {
                startOperation(source, "Play");
            }
            function pause(source) {
                startOperation(source, "Pause");
            }
            function playPause(source) {
                startOperation(source, "PlayPause");
            }
            function stop(source) {
                startOperation(source, "Stop");
            }
            function raise(source) {
                startOperation(source, "Raise");
            }
            function quit(source) {
                startOperation(source, "Quit");
            }
        }

        Loader {
            id: pulseAudio
            sourceComponent: pulseAudioComponent
            active: pulseAudioComponent.status === Component.Ready
        }

        Timer {
            id: iconGeometryTimer

            interval: 500
            repeat: false

            onTriggered: {
                tasks.publishIconGeometries(taskList.children, tasks);
            }
        }

        Binding {
            target: Plasmoid
            property: "status"
            value: (tasksModel.anyTaskDemandsAttention && Plasmoid.configuration.unhideOnAttention
                ? PlasmaCore.Types.NeedsAttentionStatus : PlasmaCore.Types.PassiveStatus)
            restoreMode: Binding.RestoreBinding
        }

        Connections {
            target: Plasmoid

            function onLocationChanged() {
                if (TaskTools.taskManagerInstanceCount >= 2) {
                    return;
                }
                // This is on a timer because the panel may not have
                // settled into position yet when the location prop-
                // erty updates.
                iconGeometryTimer.start();
            }
        }

        Connections {
            target: Plasmoid.configuration

            function onLaunchersChanged() {
                tasksModel.launcherList = Plasmoid.configuration.launchers
            }
            function onGroupingAppIdBlacklistChanged() {
                tasksModel.groupingAppIdBlacklist = Plasmoid.configuration.groupingAppIdBlacklist;
            }
            function onGroupingLauncherUrlBlacklistChanged() {
                tasksModel.groupingLauncherUrlBlacklist = Plasmoid.configuration.groupingLauncherUrlBlacklist;
            }
            function onIconSpacingChanged() {
                taskList.layout();
            }
        }

        Component {
            id: busyIndicator
            PlasmaComponents3.BusyIndicator {}
        }

        // Save drag data
        Item {
            id: dragHelper

            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction | Qt.MoveAction | Qt.LinkAction
            Drag.onDragFinished: tasks.dragSource = null;
        }

        KSvg.FrameSvgItem {
            id: taskFrame

            visible: false;

            imagePath: "widgets/tasks";
            prefix: "normal"
        }

        MouseHandler {
            id: mouseHandler

            anchors.fill: parent

            target: taskList

            onUrlsDropped: (urls) => {
                // If all dropped URLs point to application desktop files, we'll add a launcher for each of them.
                var createLaunchers = urls.every(function (item) {
                    return backend.isApplication(item)
                });

                if (createLaunchers) {
                    urls.forEach(function (item) {
                        addLauncher(item);
                    });
                    return;
                }

                if (!hoveredItem) {
                    return;
                }

                // Otherwise we'll just start a new instance of the application with the URLs as argument,
                // as you probably don't expect some of your files to open in the app and others to spawn launchers.
                tasksModel.requestOpenUrls(hoveredItem.modelIndex(), urls);
            }
        }

        ToolTipDelegate {
            id: openWindowToolTipDelegate
            visible: false
        }

        ToolTipDelegate {
            id: pinnedAppToolTipDelegate
            visible: false
        }

        TriangleMouseFilter {
            id: tmf
            filterTimeOut: 300
            active: tasks.toolTipAreaItem && tasks.toolTipAreaItem.toolTipOpen
            blockFirstEnter: false

            edge: {
                switch (Plasmoid.location) {
                    case PlasmaCore.Types.BottomEdge:
                        return Qt.TopEdge;
                    case PlasmaCore.Types.TopEdge:
                        return Qt.BottomEdge;
                    case PlasmaCore.Types.LeftEdge:
                        return Qt.RightEdge;
                    case PlasmaCore.Types.RightEdge:
                        return Qt.LeftEdge;
                    default:
                        return Qt.TopEdge;
                }
            }

            secondaryPoint: {
                if (tasks.toolTipAreaItem === null) {
                    return Qt.point(0, 0);
                }
                const x = tasks.toolTipAreaItem.x;
                const y = tasks.toolTipAreaItem.y;
                const height = tasks.toolTipAreaItem.height;
                const width = tasks.toolTipAreaItem.width;
                return Qt.point(x+width/2, height);
            }

            anchors {
                left: parent.left
                top: parent.top
            }

            height: taskList.implicitHeight
            width: taskList.implicitWidth

            TaskList {
                id: taskList

                anchors {
                    left: parent.left
                    top: parent.top
                }
                width: tasks.shouldShirnkToZero ? 0 : LayoutManager.layoutWidth()
                height: tasks.shouldShirnkToZero ? 0 : LayoutManager.layoutHeight()

                flow: {
                    if (tasks.vertical) {
                        return Plasmoid.configuration.forceStripes ? Flow.LeftToRight : Flow.TopToBottom
                    }
                    return Plasmoid.configuration.forceStripes ? Flow.TopToBottom : Flow.LeftToRight
                }

                onAnimatingChanged: {
                    if (!animating) {
                        tasks.publishIconGeometries(children, tasks);
                    }
                }
                onWidthChanged: layoutTimer.restart()
                onHeightChanged: layoutTimer.restart()

                function layout() {
                    LayoutManager.layout(taskRepeater);
                }

                Timer {
                    id: layoutTimer

                    interval: 0
                    repeat: false

                    onTriggered: taskList.layout()
                }

                Repeater {
                    id: taskRepeater

                    delegate: Task {}
                    onItemAdded: taskList.layout()
                    onItemRemoved: {
                        if (tasks.containsMouse && index != taskRepeater.count &&
                            item.m.WinIdList.length > 0 &&
                            taskClosedWithMouseMiddleButton.indexOf(item.winIdList[0]) > -1) {
                            needLayoutRefresh = true;
                        } else {
                            taskList.layout();
                        }
                        taskClosedWithMouseMiddleButton = [];
                    }
                }
            }
        }
    }

    readonly property Component groupDialogComponent: Qt.createComponent("GroupDialog.qml")
    property GroupDialog groupDialog: null

    function hasLauncher(url) {
        return tasksModel.launcherPosition(url) != -1;
    }

    function addLauncher(url) {
        if (Plasmoid.immutability !== PlasmaCore.Types.SystemImmutable) {
            tasksModel.requestAddLauncher(url);
        }
    }

    // This is called by plasmashell in response to a Meta+number shortcut.
    function activateTaskAtIndex(index) {
        if (typeof index !== "number") {
            return;
        }

        var task = taskRepeater.itemAt(index);
        if (task) {
            TaskTools.activateTask(task.modelIndex(), task.m, null, task, Plasmoid, tasks);
        }
    }

    function createContextMenu(rootTask, modelIndex, args = {}) {
        const initialArgs = Object.assign(args, {
            visualParent: rootTask,
            modelIndex,
            mpris2Source,
            backend,
        });
        return contextMenuComponent.createObject(rootTask, initialArgs);
    }

    Component.onCompleted: {
        TaskTools.taskManagerInstanceCount += 1;
        tasks.requestLayout.connect(layoutTimer.restart);
        tasks.requestLayout.connect(iconGeometryTimer.restart);
        tasks.windowsHovered.connect(backend.windowsHovered);
        tasks.activateWindowView.connect(backend.activateWindowView);
    }

    Component.onDestruction: {
        TaskTools.taskManagerInstanceCount -= 1;
    }
}
