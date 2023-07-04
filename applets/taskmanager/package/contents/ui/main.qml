/*
    SPDX-FileCopyrightText: 2012-2016 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Window
import QtQuick.Layouts 1.15
import QtQml 2.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.plasma5support 2.0 as P5Support
import org.kde.kirigami 2.20 as Kirigami

import org.kde.plasma.workspace.trianglemousefilter 1.0

import org.kde.taskmanager 0.1 as TaskManager
import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

import "code/tools.js" as TaskTools

PlasmoidItem {
    id: tasks

    // For making a bottom to top layout since qml flow can't do that.
    // We just hang the task manager upside down to achieve that.
    // This mirrors the tasks as well, so we just rotate them again to fix that (see Task.qml).
    rotation: plasmoid.configuration.reverseMode && plasmoid.formFactor === PlasmaCore.Types.Vertical ? 180 : 0

    property bool vertical: plasmoid.formFactor === PlasmaCore.Types.Vertical
    property bool iconsOnly: plasmoid.pluginName === "org.kde.plasma.icontasks"

    property var toolTipOpenedByClick: null

    property QtObject contextMenuComponent: Qt.createComponent("ContextMenu.qml")
    property QtObject pulseAudioComponent: Qt.createComponent("PulseAudio.qml")

    property var toolTipAreaItem: null

    property bool needLayoutRefresh: false;
    property variant taskClosedWithMouseMiddleButton: []
    readonly property Item taskList: Plasmoid.configuration.maxStripes === 1 ? taskLoader.item : null
    readonly property Item taskGrid: Plasmoid.configuration.maxStripes > 1 ? taskLoader.item : null

    preferredRepresentation: fullRepresentation

    Plasmoid.constraintHints: PlasmaCore.Types.CanFillArea

    Plasmoid.onUserConfiguringChanged: {
        if (plasmoid.userConfiguring && !!tasks.groupDialog) {
            tasks.groupDialog.visible = false;
        }
    }

    Layout.fillWidth: tasks.vertical ? true : (plasmoid.configuration.fill || Plasmoid.configuration.maxStripes > 1)
    Layout.fillHeight: !tasks.vertical ? true : (plasmoid.configuration.fill || Plasmoid.configuration.maxStripes > 1)
    Layout.minimumWidth: tasks.vertical || Plasmoid.configuration.maxStripes > 1 ? 0 : taskList.taskLength
    Layout.minimumHeight: !tasks.vertical || Plasmoid.configuration.maxStripes > 1 ? 0 : taskList.taskLength

    Layout.preferredWidth: Math.max(0.01, Plasmoid.configuration.maxStripes > 1 ? taskGrid.width : taskList.totalLength)
    Layout.preferredHeight: Math.max(0.01, Plasmoid.configuration.maxStripes > 1 ? taskGrid.height : taskList.totalLength)

    property Item dragSource: null

// SHARED TASK PROPERTIES BEGIN
    readonly property int spacingAdjustment: {
        if (plasmoid.pluginName === "org.kde.plasma.icontasks") {
            return Kirigami.Settings.tabletMode ? 3 : plasmoid.configuration.iconSpacing;
        }
        return 1;
    }
    readonly property real horizontalMargins: (taskFrame.margins.left + taskFrame.margins.right) * (tasks.vertical ? 1 : spacingAdjustment)
    readonly property real verticalMargins: (taskFrame.margins.left + taskFrame.margins.right) * (tasks.vertical ? 1 : spacingAdjustment)
    readonly property real iconsOnlyTaskLength: (tasks.vertical ? tasks.width + verticalMargins : tasks.height + horizontalMargins)
    readonly property real preferredMaxWidth: iconsOnlyTaskLength + (tasks.vertical || tasks.iconsOnly ? 0 : Kirigami.Units.iconSizes.sizeForLabels * 12)
// SHARED TASK PROPERTIES END

    signal windowsHovered(variant winIds, bool hovered)
    signal activateWindowView(variant winIds)

    onDragSourceChanged: {
        if (dragSource == null) {
            tasksModel.syncLaunchers();
        }
    }

    Connections {
        target: plasmoid.configuration

        function onLaunchersChanged() {
            tasksModel.launcherList = plasmoid.configuration.launchers
        }
        function onGroupingAppIdBlacklistChanged() {
            tasksModel.groupingAppIdBlacklist = plasmoid.configuration.groupingAppIdBlacklist;
        }
        function onGroupingLauncherUrlBlacklistChanged() {
            tasksModel.groupingLauncherUrlBlacklist = plasmoid.configuration.groupingLauncherUrlBlacklist;
        }
    }

    TaskManager.VirtualDesktopInfo {
        id: virtualDesktopInfo
    }

    TaskManager.ActivityInfo {
        id: activityInfo
        readonly property string nullUuid: "00000000-0000-0000-0000-000000000000"
    }

    property TaskManager.TasksModel tasksModel: TaskManager.TasksModel {
        id: tasksModel

        virtualDesktop: virtualDesktopInfo.currentDesktop
        screenGeometry: plasmoid.containment.screenGeometry
        activity: activityInfo.currentActivity

        filterByVirtualDesktop: plasmoid.configuration.showOnlyCurrentDesktop
        filterByScreen: plasmoid.configuration.showOnlyCurrentScreen
        filterByActivity: plasmoid.configuration.showOnlyCurrentActivity
        filterNotMinimized: plasmoid.configuration.showOnlyMinimized

        sortMode: sortModeEnumValue(plasmoid.configuration.sortingStrategy)
        launchInPlace: iconsOnly && plasmoid.configuration.sortingStrategy === 1
        separateLaunchers: {
            if (!iconsOnly && !plasmoid.configuration.separateLaunchers
                && plasmoid.configuration.sortingStrategy === 1) {
                return false;
            }

            return true;
        }

        groupMode: groupModeEnumValue(plasmoid.configuration.groupingStrategy)
        groupInline: !plasmoid.configuration.groupPopups && !iconsOnly
        groupingWindowTasksThreshold: (plasmoid.configuration.onlyGroupWhenFull && !iconsOnly
            ? (tasks.vertical ? tasks.height : tasks.width) / tasks.iconsOnlyTaskLength / 2 : -1)

        onLauncherListChanged: {
            plasmoid.configuration.launchers = launcherList;
        }

        onGroupingAppIdBlacklistChanged: {
            plasmoid.configuration.groupingAppIdBlacklist = groupingAppIdBlacklist;
        }

        onGroupingLauncherUrlBlacklistChanged: {
            plasmoid.configuration.groupingLauncherUrlBlacklist = groupingLauncherUrlBlacklist;
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
            launcherList = plasmoid.configuration.launchers;
            groupingAppIdBlacklist = plasmoid.configuration.groupingAppIdBlacklist;
            groupingLauncherUrlBlacklist = plasmoid.configuration.groupingLauncherUrlBlacklist;
        }
    }

    property TaskManagerApplet.Backend backend: TaskManagerApplet.Backend {
        taskManagerItem: tasks
        highlightWindows: plasmoid.configuration.highlightWindows

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

    Component {
        id: busyIndicator
        PlasmaComponents3.BusyIndicator {}
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

    Binding {
        target: plasmoid
        property: "status"
        value: (tasksModel.anyTaskDemandsAttention && plasmoid.configuration.unhideOnAttention
            ? PlasmaCore.Types.NeedsAttentionStatus : PlasmaCore.Types.PassiveStatus)
        restoreMode: Binding.RestoreBinding
    }

    Connections {
        target: Plasmoid.configuration

        function onIconSpacingChanged() {
            iconGeometryTimer.start();
        }
    }

    Connections {
        target: plasmoid

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

    Timer {
        id: iconGeometryTimer

        interval: 500
        repeat: false

        onTriggered: {
            taskLoader.item.publishIconGeometries();
        }
    }

    KSvg.FrameSvgItem {
        id: taskFrame

        visible: false;

        imagePath: "widgets/tasks";
        prefix: "normal"
    }

    ToolTipDelegate {
        id: openWindowToolTipDelegate
        visible: false
    }

    ToolTipDelegate {
        id: pinnedAppToolTipDelegate
        visible: false
    }

    MouseHandler {
        anchors.fill: parent

        target: Plasmoid.configuration.maxStripes === 1 ? taskList : taskGrid

        onUrlsDropped: (urls) => {
            // If all dropped URLs point to application desktop files, we'll add a launcher for each of them.
            const createLaunchers = urls.every(function (item) {
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

        Loader {
            id: pulseAudio
            sourceComponent: pulseAudioComponent
            active: pulseAudioComponent.status === Component.Ready
        }

        // Save drag data
        Item {
            id: dragHelper

            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction | Qt.MoveAction | Qt.LinkAction
            Drag.onDragFinished: tasks.dragSource = null;
        }

        KSvg.Svg {
            id: taskSvg

            imagePath: "widgets/tasks"
        }

        TriangleMouseFilter {
            id: tmf
            filterTimeOut: 300
            active: tasks.toolTipAreaItem && tasks.toolTipAreaItem.toolTipOpen
            blockFirstEnter: false

            edge: {
                switch (plasmoid.location) {
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

            anchors.fill: parent

            Loader {
                id: taskLoader
                anchors {
                    left: parent.left
                    top: parent.top
                }
                sourceComponent: Plasmoid.configuration.maxStripes === 1 ? taskListComponent : taskGridComponent
            }

            Component {
                id: taskListComponent

                TaskList {
                    width: tasks.vertical ? tasks.width : Math.min(taskList.totalLength, tasks.width)
                    height: tasks.vertical ? Math.min(taskList.totalLength, tasks.height) : tasks.height
                }
            }

            Component {
                id: taskGridComponent

                TaskGrid {
                    width: tasks.vertical ? tasks.width : Math.min(Screen.width, tasks.width)
                    height: tasks.vertical ? Math.min(Screen.height, tasks.height) : tasks.height
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
        if (plasmoid.immutability !== PlasmaCore.Types.SystemImmutable) {
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
            TaskTools.activateTask(task.modelIndex(), task.m, null, task, plasmoid, tasks);
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
        tasks.windowsHovered.connect(backend.windowsHovered);
        tasks.activateWindowView.connect(backend.activateWindowView);
    }

    Component.onDestruction: {
        TaskTools.taskManagerInstanceCount -= 1;
    }
}
