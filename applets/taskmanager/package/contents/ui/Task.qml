/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.draganddrop 2.0

import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

import "code/layout.js" as LayoutManager
import "code/tools.js" as TaskTools

MouseArea {
    id: task

    width: groupDialog.contentWidth
    height: Math.max(theme.mSize(theme.defaultFont).height, PlasmaCore.Units.iconSizes.medium) + LayoutManager.verticalMargins()

    visible: false

    LayoutMirroring.enabled: (Qt.application.layoutDirection == Qt.RightToLeft)
    LayoutMirroring.childrenInherit: (Qt.application.layoutDirection == Qt.RightToLeft)

    readonly property var m: model

    readonly property int pid: model.AppPid !== undefined ? model.AppPid : 0
    readonly property string appName: model.AppName
    readonly property variant winIdList: model.WinIdList
    property int itemIndex: index
    property bool inPopup: false
    property bool isWindow: model.IsWindow === true
    property int childCount: model.ChildCount !== undefined ? model.ChildCount : 0
    property int previousChildCount: 0
    property alias labelText: label.text
    property bool pressed: false
    property int pressX: -1
    property int pressY: -1
    property QtObject contextMenu: null
    property int wheelDelta: 0
    readonly property bool smartLauncherEnabled: !inPopup && model.IsStartup !== true
    property QtObject smartLauncherItem: null
    property alias toolTipAreaItem: toolTipArea
    property alias audioStreamIconLoaderItem: audioStreamIconLoader

    property Item audioStreamOverlay
    property var audioStreams: []
    property bool delayAudioStreamIndicator: false
    readonly property bool audioIndicatorsEnabled: plasmoid.configuration.indicateAudioStreams
    readonly property bool hasAudioStream: audioStreams.length > 0
    readonly property bool playingAudio: hasAudioStream && audioStreams.some(function (item) {
        return !item.corked
    })
    readonly property bool muted: hasAudioStream && audioStreams.every(function (item) {
        return item.muted
    })

    readonly property bool highlighted: (inPopup && activeFocus) || (!inPopup && containsMouse)
        || (task.contextMenu && task.contextMenu.status === PlasmaComponents.DialogStatus.Open)
        || (groupDialog.visible && groupDialog.visualParent === task)
        
    onHighlightedChanged: {
        // ensure it doesn't get stuck with a window highlighted
        backend.cancelHighlightWindows();
    }

    function showToolTip() {
        toolTipArea.showToolTip();
    }
    function hideToolTipTemporarily() {
        toolTipArea.hideToolTip();
    }

    acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MidButton | Qt.BackButton | Qt.ForwardButton

    onPidChanged: updateAudioStreams({delay: false})
    onAppNameChanged: updateAudioStreams({delay: false})

    onIsWindowChanged: {
        if (isWindow) {
            taskInitComponent.createObject(task);
        }
    }

    onChildCountChanged: {
        if (!childCount && groupDialog.visualParent == task) {
            groupDialog.visible = false;

            return;
        }

        if (containsMouse) {
            groupDialog.activeTask = null;
        }

        if (childCount > previousChildCount) {
            tasksModel.requestPublishDelegateGeometry(modelIndex(), backend.globalRect(task), task);
        }

        previousChildCount = childCount;
    }

    onItemIndexChanged: {
        hideToolTipTemporarily();

        if (!inPopup && !tasks.vertical
            && (LayoutManager.calculateStripes() > 1 || !plasmoid.configuration.separateLaunchers)) {
            tasks.requestLayout();
        }
    }

    onContainsMouseChanged:  {
        if (containsMouse) {
            if (inPopup) {
                forceActiveFocus();
            }
        } else {
            pressed = false;
        }
    }

    onPressed: {
        if (mouse.button == Qt.LeftButton || mouse.button == Qt.MidButton || mouse.button === Qt.BackButton || mouse.button === Qt.ForwardButton) {
            pressed = true;
            pressX = mouse.x;
            pressY = mouse.y;
        } else if (mouse.button == Qt.RightButton) {
            // When we're a launcher, there's no window controls, so we can show all
            // places without the menu getting super huge.
            if (model.IsLauncher === true) {
                showContextMenu({showAllPlaces: true})
            } else {
                showContextMenu();
            }
        }
    }

    onReleased: {
        if (pressed) {
            if (mouse.button == Qt.MidButton) {
                if (plasmoid.configuration.middleClickAction === TaskManagerApplet.Backend.NewInstance) {
                    tasksModel.requestNewInstance(modelIndex());
                } else if (plasmoid.configuration.middleClickAction === TaskManagerApplet.Backend.Close) {
                    tasks.taskClosedWithMouseMiddleButton = winIdList.slice()
                    tasksModel.requestClose(modelIndex());
                } else if (plasmoid.configuration.middleClickAction === TaskManagerApplet.Backend.ToggleMinimized) {
                    tasksModel.requestToggleMinimized(modelIndex());
                } else if (plasmoid.configuration.middleClickAction === TaskManagerApplet.Backend.ToggleGrouping) {
                    tasksModel.requestToggleGrouping(modelIndex());
                } else if (plasmoid.configuration.middleClickAction === TaskManagerApplet.Backend.BringToCurrentDesktop) {
                    tasksModel.requestVirtualDesktops(modelIndex(), [virtualDesktopInfo.currentDesktop]);
                }
            } else if (mouse.button == Qt.LeftButton) {
                if (plasmoid.configuration.showToolTips && toolTipArea.active) {
                    hideToolTipTemporarily();
                }
                TaskTools.activateTask(modelIndex(), model, mouse.modifiers, task);
            } else if (mouse.button === Qt.BackButton || mouse.button === Qt.ForwardButton) {
                var sourceName = mpris2Source.sourceNameForLauncherUrl(model.LauncherUrlWithoutIcon, model.AppPid);
                if (sourceName) {
                    if (mouse.button === Qt.BackButton) {
                        mpris2Source.goPrevious(sourceName);
                    } else {
                        mpris2Source.goNext(sourceName);
                    }
                } else {
                    mouse.accepted = false;
                }
            }

            backend.cancelHighlightWindows();
        }

        pressed = false;
        pressX = -1;
        pressY = -1;
    }

    onPositionChanged: {
        // mouse.button is always 0 here, hence checking with mouse.buttons
        if (pressX != -1 && mouse.buttons == Qt.LeftButton && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
            tasks.dragSource = task;
            dragHelper.startDrag(task, model.MimeType, model.MimeData,
                model.LauncherUrlWithoutIcon, model.decoration);
            pressX = -1;
            pressY = -1;

            return;
        }
    }

    onWheel: {
        if (plasmoid.configuration.wheelEnabled && (!inPopup || !groupDialog.overflowing)) {
            wheelDelta = TaskTools.wheelActivateNextPrevTask(task, wheelDelta, wheel.angleDelta.y);
        } else {
            wheel.accepted = false;
        }
    }

    onSmartLauncherEnabledChanged: {
        if (smartLauncherEnabled && !smartLauncherItem) {
            var smartLauncher = Qt.createQmlObject("
    import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet;
    TaskManagerApplet.SmartLauncherItem { }", task);

            smartLauncher.launcherUrl = Qt.binding(function() { return model.LauncherUrlWithoutIcon; });

            smartLauncherItem = smartLauncher;
        }
    }

    onHasAudioStreamChanged: {
        audioStreamIconLoader.active = hasAudioStream && audioIndicatorsEnabled;
    }

    onAudioIndicatorsEnabledChanged: {
        audioStreamIconLoader.active = hasAudioStream && audioIndicatorsEnabled;
    }

    Keys.onReturnPressed: TaskTools.activateTask(modelIndex(), model, event.modifiers, task)
    Keys.onEnterPressed: Keys.onReturnPressed(event);

    function modelIndex() {
        return (inPopup ? tasksModel.makeModelIndex(groupDialog.visualParent.itemIndex, index)
            : tasksModel.makeModelIndex(index));
    }

    function showContextMenu(args) {
        toolTipArea.hideImmediately();
        contextMenu = tasks.createContextMenu(task, modelIndex(), args);
        contextMenu.show();
    }

    function updateAudioStreams(args) {
        if (args) {
            // When the task just appeared (e.g. virtual desktop switch), show the audio indicator
            // right away. Only when audio streams change during the lifetime of this task, delay
            // showing that to avoid distraction.
            delayAudioStreamIndicator = !!args.delay;
        }

        var pa = pulseAudio.item;
        if (!pa) {
            task.audioStreams = [];
            return;
        }

        var streams = pa.streamsForPid(task.pid);
        if (streams.length) {
            pa.registerPidMatch(task.appName);
        } else {
            // We only want to fall back to appName matching if we never managed to map
            // a PID to an audio stream window. Otherwise if you have two instances of
            // an application, one playing and the other not, it will look up appName
            // for the non-playing instance and erroneously show an indicator on both.
            if (!pa.hasPidMatch(task.appName)) {
                streams = pa.streamsForAppName(task.appName);
            }
        }

        task.audioStreams = streams;
    }

    function toggleMuted() {
        if (muted) {
            task.audioStreams.forEach(function (item) { item.unmute(); });
        } else {
            task.audioStreams.forEach(function (item) { item.mute(); });
        }
    }

    Connections {
        target: pulseAudio.item
        ignoreUnknownSignals: true // Plasma-PA might not be available
        function onStreamsChanged() { 
            task.updateAudioStreams({delay: true})
        }
    }

    Component {
        id: taskInitComponent

        Timer {
            id: timer

            interval: PlasmaCore.Units.longDuration
            repeat: false

            onTriggered: {
                parent.hoverEnabled = true;

                if (parent.isWindow) {
                    tasksModel.requestPublishDelegateGeometry(parent.modelIndex(),
                        backend.globalRect(parent), parent);
                }

                timer.destroy();
            }

            Component.onCompleted: timer.start()
        }
    }

    PlasmaCore.FrameSvgItem {
        id: frame

        anchors {
            fill: parent

            topMargin: (!tasks.vertical && taskList.rows > 1) ? PlasmaCore.Units.smallSpacing / 4 : 0
            bottomMargin: (!tasks.vertical && taskList.rows > 1) ? PlasmaCore.Units.smallSpacing / 4 : 0
            leftMargin: ((inPopup || tasks.vertical) && taskList.columns > 1) ? PlasmaCore.Units.smallSpacing / 4 : 0
            rightMargin: ((inPopup || tasks.vertical) && taskList.columns > 1) ? PlasmaCore.Units.smallSpacing / 4 : 0
        }

        imagePath: "widgets/tasks"
        property bool isHovered: task.highlighted && plasmoid.configuration.taskHoverEffect
        property string basePrefix: "normal"
        prefix: isHovered ? TaskTools.taskPrefixHovered(basePrefix) : TaskTools.taskPrefix(basePrefix)

        PlasmaCore.ToolTipArea {
            id: toolTipArea

            anchors.fill: parent
            location: plasmoid.location

            active: !inPopup && !groupDialog.visible && plasmoid.configuration.showToolTips
            interactive: model.IsWindow === true || mainItem.hasPlayer

            mainItem: (model.IsWindow === true) ? openWindowToolTipDelegate : pinnedAppToolTipDelegate

            onContainsMouseChanged:  {
                if (containsMouse) {
                    // Only assign different values to mainItem to avoid unnecessary reevaluation
                    if (mainItem.parentTask !== task) {
                        mainItem.parentTask = task;
                    }

                    let rootIndex = tasksModel.makeModelIndex(itemIndex, -1);

                    if (mainItem.rootIndex !== rootIndex) {
                        mainItem.rootIndex = rootIndex;
                    }

                    mainItem.appName = Qt.binding(function() {
                        return model.AppName;
                    });
                    mainItem.pidParent = Qt.binding(function() {
                        return model.AppPid !== undefined ? model.AppPid : 0;
                    });
                    mainItem.windows = Qt.binding(function() {
                        return model.WinIdList;
                    });
                    mainItem.isGroup = Qt.binding(function() {
                        return model.IsGroupParent === true;
                    });
                    mainItem.icon = Qt.binding(function() {
                        return model.decoration;
                    });
                    mainItem.launcherUrl = Qt.binding(function() {
                        return model.LauncherUrlWithoutIcon;
                    });
                    mainItem.isLauncher = Qt.binding(function() {
                        return model.IsLauncher === true;
                    });
                    mainItem.isMinimizedParent = Qt.binding(function() {
                        return model.IsMinimized === true;
                    });
                    mainItem.displayParent = Qt.binding(function() {
                        return model.display;
                    });
                    mainItem.genericName = Qt.binding(function() {
                        return model.GenericName;
                    });
                    mainItem.virtualDesktopParent = Qt.binding(function() {
                        return (model.VirtualDesktops !== undefined && model.VirtualDesktops.length > 0) ? model.VirtualDesktops : [0];
                    });
                    mainItem.isOnAllVirtualDesktopsParent = Qt.binding(function() {
                        return model.IsOnAllVirtualDesktops === true;
                    });
                    mainItem.activitiesParent = Qt.binding(function() {
                        return model.Activities;
                    });

                    mainItem.smartLauncherCountVisible = Qt.binding(function() {
                        return task.smartLauncherItem && task.smartLauncherItem.countVisible;
                    });
                    mainItem.smartLauncherCount = Qt.binding(function() {
                        return mainItem.smartLauncherCountVisible ? task.smartLauncherItem.count : 0;
                    });
                }
            }
        }
    }

    Loader {
        anchors.fill: frame
        asynchronous: true
        source: "TaskProgressOverlay.qml"
        active: task.isWindow && task.smartLauncherItem && task.smartLauncherItem.progressVisible
    }

    Item {
        id: iconBox

        anchors {
            left: parent.left
            leftMargin: adjustMargin(true, parent.width, taskFrame.margins.left)
            top: parent.top
            topMargin: adjustMargin(false, parent.height, taskFrame.margins.top)
        }

        width: height
        height: (parent.height - adjustMargin(false, parent.height, taskFrame.margins.top)
            - adjustMargin(false, parent.height, taskFrame.margins.bottom))

        function adjustMargin(vert, size, margin) {
            if (!size) {
                return margin;
            }

            var margins = vert ? LayoutManager.horizontalMargins() : LayoutManager.verticalMargins();

            if ((size - margins) < PlasmaCore.Units.iconSizes.small) {
                return Math.ceil((margin * (PlasmaCore.Units.iconSizes.small / size)) / 2);
            }

            return margin;
        }

        //width: inPopup ? PlasmaCore.Units.iconSizes.small : Math.min(height, parent.width - LayoutManager.horizontalMargins())

        PlasmaCore.IconItem {
            id: icon

            anchors.fill: parent

            active: task.highlighted
            enabled: true
            usesPlasmaTheme: false

            source: model.decoration
        }

        Loader {
            // QTBUG anchors.fill in conjunction with the Loader doesn't reliably work on creation:
            // have a window with a badge, move it from one screen to another, the new task item on the
            // other screen will now have a glitched out badge mask.
            width: parent.width
            height: parent.height
            asynchronous: true
            source: "TaskBadgeOverlay.qml"
            active: height >= PlasmaCore.Units.iconSizes.small
                    && task.smartLauncherItem && task.smartLauncherItem.countVisible
        }

        states: [
            // Using a state transition avoids a binding loop between label.visible and
            // the text label margin, which derives from the icon width.
            State {
                name: "standalone"
                when: !label.visible

                AnchorChanges {
                    target: iconBox
                    anchors.left: undefined
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                PropertyChanges {
                    target: iconBox
                    anchors.leftMargin: 0
                    width: parent.width - adjustMargin(true, task.width, taskFrame.margins.left)
                                        - adjustMargin(true, task.width, taskFrame.margins.right)
                }
            }
        ]

        Loader {
            anchors.fill: parent

            active: model.IsStartup === true
            sourceComponent: busyIndicator
        }

        Component {
            id: busyIndicator

            PlasmaComponents.BusyIndicator { anchors.fill: parent }
        }
    }

    Loader {
        id: audioStreamIconLoader

        readonly property bool shown: item && item.visible
        readonly property var indicatorScale: 1.2

        source: "AudioStream.qml"
        width: Math.min(Math.min(iconBox.width, iconBox.height) * 0.4, PlasmaCore.Units.iconSizes.smallMedium)
        height: width

        anchors {
            right: frame.right
            top: frame.top
            rightMargin: taskFrame.margins.right
            topMargin: Math.round(taskFrame.margins.top * indicatorScale)
        }
    }

    PlasmaComponents.Label {
        id: label

        visible: (inPopup || !iconsOnly && model.IsLauncher !== true
            && (parent.width - iconBox.height - PlasmaCore.Units.smallSpacing) >= (theme.mSize(theme.defaultFont).width * LayoutManager.minimumMColumns()))

        anchors {
            fill: parent
            leftMargin: taskFrame.margins.left + iconBox.width + PlasmaCore.Units.smallSpacing
            topMargin: taskFrame.margins.top
            rightMargin: taskFrame.margins.right + (audioStreamIconLoader.shown ? (audioStreamIconLoader.width + PlasmaCore.Units.smallSpacing) : 0)
            bottomMargin: taskFrame.margins.bottom
        }

        text: model.display
        wrapMode: (maximumLineCount == 1) ? Text.NoWrap : Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.PlainText
        verticalAlignment: Text.AlignVCenter
        maximumLineCount: plasmoid.configuration.maxTextLines || undefined
    }

    states: [
        State {
            name: "launcher"
            when: model.IsLauncher === true

            PropertyChanges {
                target: frame
                basePrefix: ""
            }
        },
        State {
            name: "attention"
            when: model.IsDemandingAttention === true || (task.smartLauncherItem && task.smartLauncherItem.urgent)

            PropertyChanges {
                target: frame
                basePrefix: "attention"
            }
        },
        State {
            name: "minimized"
            when: model.IsMinimized === true

            PropertyChanges {
                target: frame
                basePrefix: "minimized"
            }
        },
        State {
            name: "active"
            when: model.IsActive === true

            PropertyChanges {
                target: frame
                basePrefix: "focus"
            }
        }
    ]

    Component.onCompleted: {
        if (!inPopup && model.IsWindow === true) {
            var component = Qt.createComponent("GroupExpanderOverlay.qml");
            component.createObject(task);
        }

        if (!inPopup && model.IsWindow !== true) {
            taskInitComponent.createObject(task);
        }

        updateAudioStreams({delay: false})
    }
}
