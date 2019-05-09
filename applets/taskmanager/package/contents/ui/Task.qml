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

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.draganddrop 2.0

import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

import "code/layout.js" as LayoutManager
import "code/tools.js" as TaskTools

MouseArea {
    id: task

    width: groupDialog.contentWidth
    height: Math.max(theme.mSize(theme.defaultFont).height, units.iconSizes.medium) + LayoutManager.verticalMargins()

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

    property Item audioStreamOverlay
    property var audioStreams: []
    property bool delayAudioStreamIndicator: false
    readonly property bool hasAudioStream: plasmoid.configuration.indicateAudioStreams && audioStreams.length > 0
    readonly property bool playingAudio: hasAudioStream && audioStreams.some(function (item) {
        return !item.corked
    })
    readonly property bool muted: hasAudioStream && audioStreams.every(function (item) {
        return item.muted
    })

    readonly property bool highlighted: (inPopup && activeFocus) || (!inPopup && containsMouse)
        || (task.contextMenu && task.contextMenu.status === PlasmaComponents.DialogStatus.Open)
        || (groupDialog.visible && groupDialog.visualParent === task)

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

        if (model.IsWindow === true) {
            tasks.windowsHovered(model.WinIdList, containsMouse);
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
                TaskTools.activateTask(modelIndex(), model, mouse.modifiers, task);
                if (plasmoid.configuration.showToolTips) {
                    hideToolTipTemporarily();
                }
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
        if (hasAudioStream) {
            audioStreamIconLoader.active = true
        }
    }

    Keys.onReturnPressed: TaskTools.activateTask(modelIndex(), model, event.modifiers, task)
    Keys.onEnterPressed: Keys.onReturnPressed(event);

    function modelIndex() {
        return (inPopup ? tasksModel.makeModelIndex(groupDialog.visualParent.itemIndex, index)
            : tasksModel.makeModelIndex(index));
    }

    function showContextMenu(args) {
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
        onStreamsChanged: task.updateAudioStreams({delay: true})
    }

    Component {
        id: taskInitComponent

        Timer {
            id: timer

            interval: units.longDuration * 2
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

            topMargin: (!tasks.vertical && taskList.rows > 1) ? units.smallSpacing / 4 : 0
            bottomMargin: (!tasks.vertical && taskList.rows > 1) ? units.smallSpacing / 4 : 0
            leftMargin: ((inPopup || tasks.vertical) && taskList.columns > 1) ? units.smallSpacing / 4 : 0
            rightMargin: ((inPopup || tasks.vertical) && taskList.columns > 1) ? units.smallSpacing / 4 : 0
        }

        imagePath: "widgets/tasks"
        property string basePrefix: "normal"
        prefix: TaskTools.taskPrefix(basePrefix)

        PlasmaCore.ToolTipArea {
            id: toolTipArea

            anchors.fill: parent
            location: plasmoid.location

            active: !inPopup && !groupDialog.visible && plasmoid.configuration.showToolTips
            interactive: true

            mainItem: toolTipDelegate

            onContainsMouseChanged:  {
                if (containsMouse) {
                    toolTipDelegate.parentTask = task;
                    toolTipDelegate.rootIndex = tasksModel.makeModelIndex(itemIndex, -1);

                    toolTipDelegate.appName = Qt.binding(function() {
                        return model.AppName;
                    });
                    toolTipDelegate.pidParent = Qt.binding(function() {
                        return model.AppPid;
                    });
                    toolTipDelegate.windows = Qt.binding(function() {
                        return model.WinIdList;
                    });
                    toolTipDelegate.isGroup = Qt.binding(function() {
                        return model.IsGroupParent === true;
                    });
                    toolTipDelegate.icon = Qt.binding(function() {
                        return model.decoration;
                    });
                    toolTipDelegate.launcherUrl = Qt.binding(function() {
                        return model.LauncherUrlWithoutIcon;
                    });
                    toolTipDelegate.isLauncher = Qt.binding(function() {
                        return model.IsLauncher === true;
                    });
                    toolTipDelegate.isMinimizedParent = Qt.binding(function() {
                        return model.IsMinimized === true;
                    });
                    toolTipDelegate.displayParent = Qt.binding(function() {
                        return model.display;
                    });
                    toolTipDelegate.genericName = Qt.binding(function() {
                        return model.GenericName;
                    });
                    toolTipDelegate.virtualDesktopParent = Qt.binding(function() {
                        return (model.VirtualDesktops !== undefined || model.VirtualDesktops.length === 0) ? model.VirtualDesktops : [0];
                    });
                    toolTipDelegate.isOnAllVirtualDesktopsParent = Qt.binding(function() {
                        return model.IsOnAllVirtualDesktops === true;
                    });
                    toolTipDelegate.activitiesParent = Qt.binding(function() {
                        return model.Activities;
                    });

                    toolTipDelegate.smartLauncherCountVisible = Qt.binding(function() {
                        return task.smartLauncherItem && task.smartLauncherItem.countVisible;
                    });
                    toolTipDelegate.smartLauncherCount = Qt.binding(function() {
                        return toolTipDelegate.smartLauncherCountVisible ? task.smartLauncherItem.count : 0;
                    });
                }
            }
        }
    }

    Loader {
        anchors.fill: frame
        asynchronous: true
        source: "TaskProgressOverlay.qml"
        active: task.smartLauncherItem && task.smartLauncherItem.progressVisible
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

            if ((size - margins) < units.iconSizes.small) {
                return Math.ceil((margin * (units.iconSizes.small / size)) / 2);
            }

            return margin;
        }

        //width: inPopup ? units.iconSizes.small : Math.min(height, parent.width - LayoutManager.horizontalMargins())

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
            active: height >= units.iconSizes.small
                    && task.smartLauncherItem && task.smartLauncherItem.countVisible
        }

        states: [
            // Using a state transition avoids a binding loop between label.visible and
            // the text label margin, which derives from the icon width.
            State {
                name: "standalone"
                when: !label.visible && !audioStreamIconLoader.shown

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

        source: "AudioStream.qml"
        width: units.roundToIconSize(Math.min(Math.min(iconBox.width, iconBox.height), units.iconSizes.smallMedium))
        height: width

        anchors {
            right: parent.right
            rightMargin: iconBox.adjustMargin(true, parent.width, taskFrame.margins.right)
            verticalCenter: parent.verticalCenter
        }
    }

    PlasmaComponents.Label {
        id: label

        visible: (inPopup || !iconsOnly && model.IsLauncher !== true
            && (parent.width - iconBox.height - units.smallSpacing) >= (theme.mSize(theme.defaultFont).width * LayoutManager.minimumMColumns()))

        anchors {
            fill: parent
            leftMargin: taskFrame.margins.left + iconBox.width + units.smallSpacing
            topMargin: taskFrame.margins.top
            rightMargin: taskFrame.margins.right + (audioStreamIconLoader.shown ? (audioStreamIconLoader.width + units.smallSpacing) : 0)
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
            name: "hovered"
            when: task.highlighted && frame.hasElementPrefix("hover") && plasmoid.configuration.taskHoverEffect

            PropertyChanges {
                target: frame
                basePrefix: "hover"
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
