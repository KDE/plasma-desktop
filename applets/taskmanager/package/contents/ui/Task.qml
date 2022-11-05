/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents // for DialogStatus
import org.kde.plasma.components 3.0 as PlasmaComponents3

import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

import "code/layout.js" as LayoutManager
import "code/tools.js" as TaskTools

PlasmaCore.ToolTipArea {
    id: task

    activeFocusOnTab: true

    height: Math.max(theme.mSize(theme.defaultFont).height, PlasmaCore.Units.iconSizes.medium) + LayoutManager.verticalMargins()

    visible: false

    // To achieve a bottom to top layout, the task manager is rotated by 180 degrees(see main.qml).
    // This makes the tasks mirrored, so we mirror them again to fix that.
    rotation: plasmoid.configuration.reverseMode && plasmoid.formFactor === PlasmaCore.Types.Vertical ? 180 : 0

    LayoutMirroring.enabled: (Qt.application.layoutDirection == Qt.RightToLeft)
    LayoutMirroring.childrenInherit: (Qt.application.layoutDirection == Qt.RightToLeft)

    readonly property var m: model

    readonly property int pid: model.AppPid !== undefined ? model.AppPid : 0
    readonly property string appName: model.AppName || ""
    readonly property string appId: model.AppId.replace(/\.desktop/, '')
    readonly property variant winIdList: model.WinIdList
    property int itemIndex: index
    property bool inPopup: false
    property bool isWindow: model.IsWindow === true
    property int childCount: model.ChildCount !== undefined ? model.ChildCount : 0
    property int previousChildCount: 0
    property alias labelText: label.text
    property QtObject contextMenu: null
    readonly property bool smartLauncherEnabled: !inPopup && model.IsStartup !== true
    property QtObject smartLauncherItem: null

    property Item audioStreamIcon: null
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
        || (!!tasks.groupDialog && tasks.groupDialog.visualParent === task)

    active: (plasmoid.configuration.showToolTips || tasks.toolTipOpenedByClick === task) && !inPopup && !tasks.groupDialog
    interactive: model.IsWindow === true || mainItem.hasPlayer
    location: plasmoid.location
    mainItem: (model.IsWindow === true) ? openWindowToolTipDelegate : pinnedAppToolTipDelegate
    // when the mouse leaves the tooltip area, a timer to hide is set for (timeout / 20) ms
    // see plasma-framework/src/declarativeimports/core/tooltipdialog.cpp function dismiss()
    // to compensate for that we multiply by 20 here, to get an effective leave timeout of 2s.
    timeout: (tasks.toolTipOpenedByClick === task) ? 2000 * 20 : 4000

    Accessible.name: model.display
    Accessible.description: {
        if (!model.display) {
            return "";
        }

        if (model.IsLauncher) {
            return i18nc("@info:usagetip %1 application name", "Launch %1", model.display)
        }

        let smartLauncherDescription = "";
        if (iconBox.active) {
            smartLauncherDescription += i18ncp("@info:tooltip", "There is %1 new message.", "There are %1 new messages.", task.smartLauncherItem.count);
        }

        if (model.IsGroupParent) {
            switch (plasmoid.configuration.groupedTaskVisualization) {
            case 0:
                break; // Use the default description
            case 1: {
                if (plasmoid.configuration.showToolTips) {
                    return `${i18nc("@info:usagetip %1 task name", "Show Task tooltip for %1", model.display)}; ${smartLauncherDescription}`;
                }
                // fallthrough
            }
            case 2: {
                if (backend.windowViewAvailable) {
                    return `${i18nc("@info:usagetip %1 task name", "Show windows side by side for %1", model.display)}; ${smartLauncherDescription}`;
                }
                // fallthrough
            }
            default:
                return `${i18nc("@info:usagetip %1 task name", "Open textual list of windows for %1", model.display)}; ${smartLauncherDescription}`;
            }
        }

        return `${i18n("Activate %1", model.display)}; ${smartLauncherDescription}`;
    }
    Accessible.role: Accessible.Button

    onToolTipVisibleChanged: {
        if (!toolTipVisible) {
            tasks.toolTipOpenedByClick = null;
        }
    }

    onContainsMouseChanged: if (containsMouse) {
        task.forceActiveFocus(Qt.MouseFocusReason);
        task.updateMainItemBindings();
    } else {
        tasks.toolTipOpenedByClick = null;
    }

    onHighlightedChanged: {
        // ensure it doesn't get stuck with a window highlighted
        backend.cancelHighlightWindows();
    }

    onPidChanged: updateAudioStreams({delay: false})
    onAppNameChanged: updateAudioStreams({delay: false})

    onIsWindowChanged: {
        if (isWindow) {
            taskInitComponent.createObject(task);
        }
    }

    onChildCountChanged: {
        if (TaskTools.taskManagerInstanceCount < 2 && childCount > previousChildCount) {
            tasksModel.requestPublishDelegateGeometry(modelIndex(), backend.globalRect(task), task);
        }

        previousChildCount = childCount;
    }

    onItemIndexChanged: {
        hideToolTip();

        if (!inPopup && !tasks.vertical
            && (LayoutManager.calculateStripes() > 1 || !plasmoid.configuration.separateLaunchers)) {
            tasks.requestLayout();
        }
    }

    onSmartLauncherEnabledChanged: {
        if (smartLauncherEnabled && !smartLauncherItem) {
            const smartLauncher = Qt.createQmlObject(`
                import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet;

                TaskManagerApplet.SmartLauncherItem { }
            `, task);

            smartLauncher.launcherUrl = Qt.binding(() => model.LauncherUrlWithoutIcon);

            smartLauncherItem = smartLauncher;
        }
    }

    onHasAudioStreamChanged: {
        const audioStreamIconActive = hasAudioStream && audioIndicatorsEnabled;
        if (!audioStreamIconActive) {
            if (audioStreamIcon !== null) {
                audioStreamIcon.destroy();
                audioStreamIcon = null;
            }
            return;
        }
        // Create item on demand instead of using Loader to reduce memory consumption,
        // because only a few applications have audio streams.
        const component = Qt.createComponent("AudioStream.qml");
        audioStreamIcon = component.createObject(task);
        component.destroy();
    }
    onAudioIndicatorsEnabledChanged: task.hasAudioStreamChanged()

    Keys.onReturnPressed: TaskTools.activateTask(modelIndex(), model, event.modifiers, task, plasmoid, tasks)
    Keys.onEnterPressed: Keys.onReturnPressed(event);
    Keys.onSpacePressed: Keys.onReturnPressed(event);
    Keys.onUpPressed: Keys.onLeftPressed(event)
    Keys.onDownPressed: Keys.onRightPressed(event)
    Keys.onLeftPressed: if (!inPopup && (event.modifiers & Qt.ControlModifier) && (event.modifiers & Qt.ShiftModifier)) {
        tasksModel.move(task.itemIndex, task.itemIndex - 1);
    } else {
        event.accepted = false;
    }
    Keys.onRightPressed: if (!inPopup && (event.modifiers & Qt.ControlModifier) && (event.modifiers & Qt.ShiftModifier)) {
        tasksModel.move(task.itemIndex, task.itemIndex + 1);
    } else {
        event.accepted = false;
    }

    function modelIndex() {
        return (inPopup ? tasksModel.makeModelIndex(groupDialog.visualParent.itemIndex, index)
            : tasksModel.makeModelIndex(index));
    }

    function showContextMenu(args) {
        task.hideImmediately();
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

        // Check appid first for app using portal
        // https://docs.pipewire.org/page_portal.html
        var streams = pa.streamsForAppId(task.appId);
        if (!streams.length) {
            streams = pa.streamsForPid(task.pid);
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

    // Will also be called in activateTaskAtIndex(index)
    function updateMainItemBindings() {
        if (tasks.toolTipOpenedByClick !== null && tasks.toolTipOpenedByClick !== task) {
            return;
        }

        mainItem.parentTask = task;
        mainItem.rootIndex = tasksModel.makeModelIndex(itemIndex, -1);

        mainItem.appName = Qt.binding(() => model.AppName);
        mainItem.pidParent = Qt.binding(() => model.AppPid !== undefined ? model.AppPid : 0);
        mainItem.windows = Qt.binding(() => model.WinIdList);
        mainItem.isGroup = Qt.binding(() => model.IsGroupParent === true);
        mainItem.icon = Qt.binding(() => model.decoration);
        mainItem.launcherUrl = Qt.binding(() => model.LauncherUrlWithoutIcon);
        mainItem.isLauncher = Qt.binding(() => model.IsLauncher === true);
        mainItem.isMinimizedParent = Qt.binding(() => model.IsMinimized === true);
        mainItem.displayParent = Qt.binding(() => model.display);
        mainItem.genericName = Qt.binding(() => model.GenericName);
        mainItem.virtualDesktopParent = Qt.binding(() =>
            (model.VirtualDesktops !== undefined && model.VirtualDesktops.length > 0) ? model.VirtualDesktops : [0]);
        mainItem.isOnAllVirtualDesktopsParent = Qt.binding(() => model.IsOnAllVirtualDesktops === true);
        mainItem.activitiesParent = Qt.binding(() => model.Activities);

        mainItem.smartLauncherCountVisible = Qt.binding(() => task.smartLauncherItem && task.smartLauncherItem.countVisible);
        mainItem.smartLauncherCount = Qt.binding(() => mainItem.smartLauncherCountVisible ? task.smartLauncherItem.count : 0);
    }

    Connections {
        target: pulseAudio.item
        ignoreUnknownSignals: true // Plasma-PA might not be available
        function onStreamsChanged() {
            task.updateAudioStreams({delay: true})
        }
    }

    TapHandler {
        id: menuTapHandler
        acceptedButtons: Qt.LeftButton
        acceptedDevices: PointerDevice.TouchScreen | PointerDevice.Stylus
        onLongPressed: {
            // When we're a launcher, there's no window controls, so we can show all
            // places without the menu getting super huge.
            if (model.IsLauncher === true) {
                showContextMenu({showAllPlaces: true})
            } else {
                showContextMenu();
            }
        }
    }

    TapHandler {
        acceptedButtons: Qt.RightButton
        acceptedDevices: PointerDevice.Mouse
        onTapped: menuTapHandler.longPressed();
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onTapped: {
            if (plasmoid.configuration.showToolTips && task.active) {
                hideToolTip();
            }
            TaskTools.activateTask(modelIndex(), model, eventPoint.event.modifiers, task, plasmoid, tasks);
        }
    }

    TapHandler {
        acceptedButtons: Qt.MidButton | Qt.BackButton | Qt.ForwardButton
        onTapped: {
            const button = eventPoint.event.button;
            if (button == Qt.MidButton) {
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
            } else if (button === Qt.BackButton || button === Qt.ForwardButton) {
                var sourceName = mpris2Source.sourceNameForLauncherUrl(model.LauncherUrlWithoutIcon, model.AppPid);
                if (sourceName) {
                    if (button === Qt.BackButton) {
                        mpris2Source.goPrevious(sourceName);
                    } else {
                        mpris2Source.goNext(sourceName);
                    }
                } else {
                    eventPoint.accepted = false;
                }
            }

            backend.cancelHighlightWindows();
        }
    }

    WheelHandler {
        property int wheelDelta: 0
        enabled: plasmoid.configuration.wheelEnabled && (!task.inPopup || !groupDialog.overflowing)
        onWheel: {
            wheelDelta = TaskTools.wheelActivateNextPrevTask(task, wheelDelta, event.angleDelta.y, plasmoid.configuration.wheelSkipMinimized, tasks);
        }
    }

    PlasmaCore.FrameSvgItem {
        id: frame

        anchors {
            fill: parent

            topMargin: (!tasks.vertical && taskList.rows > 1) ? LayoutManager.iconMargin : 0
            bottomMargin: (!tasks.vertical && taskList.rows > 1) ? LayoutManager.iconMargin : 0
            leftMargin: ((inPopup || tasks.vertical) && taskList.columns > 1) ? LayoutManager.iconMargin : 0
            rightMargin: ((inPopup || tasks.vertical) && taskList.columns > 1) ? LayoutManager.iconMargin : 0
        }

        imagePath: "widgets/tasks"
        property bool isHovered: task.highlighted && plasmoid.configuration.taskHoverEffect
        property string basePrefix: "normal"
        prefix: isHovered ? TaskTools.taskPrefixHovered(basePrefix, plasmoid.location) : TaskTools.taskPrefix(basePrefix, plasmoid.location)

        // Avoid repositioning delegate item after dragFinished
        DragHandler {
            id: dragHandler

            onActiveChanged: if (active) {
                icon.grabToImage((result) => {
                    tasks.dragSource = task;
                    dragHelper.Drag.imageSource = result.url;
                    dragHelper.Drag.mimeData = dragHelper.generateMimeData(model.MimeType, model.MimeData, model.LauncherUrlWithoutIcon);
                    dragHelper.Drag.active = dragHandler.active;
                });
            } else {
                dragHelper.Drag.active = false;
                dragHelper.Drag.imageSource = "";
            }
        }
    }

    Loader {
        id: taskProgressOverlayLoader

        anchors.fill: frame
        asynchronous: true
        active: task.isWindow && task.smartLauncherItem && task.smartLauncherItem.progressVisible

        sourceComponent: TaskProgressOverlay {
            from: 0
            to: 100
            value: task.smartLauncherItem.progress
        }
    }

    Loader {
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

        asynchronous: true
        active: height >= PlasmaCore.Units.iconSizes.small
                && task.smartLauncherItem && task.smartLauncherItem.countVisible
        source: "TaskBadgeOverlay.qml"

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

        PlasmaCore.IconItem {
            id: icon

            anchors.fill: parent

            active: task.highlighted
            enabled: true
            usesPlasmaTheme: false

            source: model.decoration
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
            anchors.centerIn: parent
            width: Math.min(parent.width, parent.height)
            height: width
            active: model.IsStartup === true
            sourceComponent: busyIndicator
        }
    }

    PlasmaComponents3.Label {
        id: label

        visible: (inPopup || !iconsOnly && model.IsLauncher !== true
            && (parent.width - iconBox.height - PlasmaCore.Units.smallSpacing) >= (theme.mSize(theme.defaultFont).width * LayoutManager.minimumMColumns()))

        anchors {
            fill: parent
            leftMargin: taskFrame.margins.left + iconBox.width + LayoutManager.labelMargin
            topMargin: taskFrame.margins.top
            rightMargin: taskFrame.margins.right + (audioStreamIcon !== null && audioStreamIcon.visible ? (audioStreamIcon.width + LayoutManager.labelMargin) : 0)
            bottomMargin: taskFrame.margins.bottom
        }

        wrapMode: (maximumLineCount == 1) ? Text.NoWrap : Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.PlainText
        verticalAlignment: Text.AlignVCenter
        maximumLineCount: plasmoid.configuration.maxTextLines || undefined

        // use State to avoid unnecessary re-evaluation when the label is invisible
        states: State {
            name: "labelVisible"
            when: label.visible

            PropertyChanges {
                target: label
                text: model.display || ""
            }
        }
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
            component.destroy();
        }

        if (!inPopup && model.IsWindow !== true) {
            taskInitComponent.createObject(task);
        }

        updateAudioStreams({delay: false})
    }
}
