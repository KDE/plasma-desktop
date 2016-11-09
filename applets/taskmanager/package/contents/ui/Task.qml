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

import "../code/layout.js" as LayoutManager
import "../code/tools.js" as TaskTools

MouseArea {
    id: task

    width: groupDialog.mainItem.width
    height: Math.max(theme.mSize(theme.defaultFont).height, units.iconSizes.small) + LayoutManager.verticalMargins()

    visible: false

    LayoutMirroring.enabled: (Qt.application.layoutDirection == Qt.RightToLeft)
    LayoutMirroring.childrenInherit: (Qt.application.layoutDirection == Qt.RightToLeft)

    readonly property var m: model

    property int itemIndex: index
    property bool inPopup: false
    property bool isWindow: model.IsWindow === true
    property alias textWidth: label.implicitWidth
    property bool pressed: false
    property int pressX: -1
    property int pressY: -1
    property QtObject contextMenu: null
    property int wheelDelta: 0
    readonly property bool smartLauncherEnabled: plasmoid.configuration.smartLaunchersEnabled && !inPopup && model.IsStartup !== true
    property QtObject smartLauncherItem: null

    readonly property bool highlighted: (inPopup && activeFocus) || (!inPopup && containsMouse)

    acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MidButton

    onIsWindowChanged: {
        if (isWindow) {
            taskInitComponent.createObject(task);
        }
    }

    onItemIndexChanged: {
        if (!inPopup && !tasks.vertical
            && (LayoutManager.calculateStripes() > 1 || !plasmoid.configuration.separateLaunchers)) {
            tasks.requestLayout();
        }
    }

    onContainsMouseChanged:  {
        if (containsMouse) {
            if (inPopup) {
                forceActiveFocus()
            }
        } else {
            pressed = false;
        }

        if (model.IsWindow === true) {
            tasks.windowsHovered(model.LegacyWinIdList, containsMouse);
        }
    }

    onPressed: {
        if (mouse.button == Qt.LeftButton || mouse.button == Qt.MidButton) {
            pressed = true;
            pressX = mouse.x;
            pressY = mouse.y;
        } else if (mouse.button == Qt.RightButton) {
            if (plasmoid.configuration.showToolTips) {
                toolTip.hideToolTip();
            }

            tasks.createContextMenu(task).show();
        }
    }

    onReleased: {
        if (pressed) {
            if (mouse.button == Qt.MidButton) {
                if (plasmoid.configuration.middleClickAction == TaskManagerApplet.Backend.NewInstance) {
                    tasksModel.requestNewInstance(modelIndex());
                } else if (plasmoid.configuration.middleClickAction == TaskManagerApplet.Backend.Close) {
                    tasksModel.requestClose(modelIndex());
                } else if (plasmoid.configuration.middleClickAction == TaskManagerApplet.Backend.ToggleMinimized) {
                    tasksModel.requestToggleMinimized(modelIndex());
                }
            } else if (mouse.button == Qt.LeftButton) {
                TaskTools.activateTask(modelIndex(), model, mouse.modifiers);
            }
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
        if (plasmoid.configuration.wheelEnabled) {
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

    Keys.onReturnPressed: TaskTools.activateTask(modelIndex(), model, event.modifiers)
    Keys.onEnterPressed: Keys.onReturnPressed(event);

    function modelIndex() {
        return (inPopup ? tasksModel.makeModelIndex(groupDialog.visualParent.itemIndex, index)
            : tasksModel.makeModelIndex(index));
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
        onRepaintNeeded: updatePrefix()

        function updatePrefix() {
            prefix = Qt.binding(function() {
                return TaskTools.taskPrefix(basePrefix);
            });
        }

        PlasmaCore.ToolTipArea {
            id: toolTip

            anchors.fill: parent

            active: !inPopup && !groupDialog.visible && plasmoid.configuration.showToolTips
            interactive: true
            location: plasmoid.location

            mainItem: toolTipDelegate

            onContainsMouseChanged:  {
                if (containsMouse) {
                    toolTipDelegate.parentIndex = itemIndex;

                    toolTipDelegate.windows = Qt.binding(function() {
                        return model.LegacyWinIdList;
                    });
                    toolTipDelegate.mainText = Qt.binding(function() {
                        return model.display;
                    });
                    toolTipDelegate.icon = Qt.binding(function() {
                        return model.decoration;
                    });
                    toolTipDelegate.subText = Qt.binding(function() {
                        return model.IsLauncher === true ? model.GenericName : toolTip.generateSubText(model);
                    });
                    toolTipDelegate.launcherUrl = Qt.binding(function() {
                        return model.LauncherUrlWithoutIcon;
                    });
                }
            }

            function generateSubText(task) {
                var subTextEntries = new Array();

                if (!plasmoid.configuration.showOnlyCurrentDesktop
                    && virtualDesktopInfo.numberOfDesktops > 1
                    && model.IsOnAllVirtualDesktops !== true
                    && model.VirtualDesktop != -1
                    && model.VirtualDesktop != undefined) {
                    subTextEntries.push(i18n("On %1", virtualDesktopInfo.desktopNames[model.VirtualDesktop - 1]));
                }

                if (model.Activities == undefined) {
                    return subTextEntries.join("\n");
                }

                if (model.Activities.length == 0 && activityInfo.numberOfRunningActivities > 1) {
                    subTextEntries.push(i18nc("Which virtual desktop a window is currently on",
                        "Available on all activities"));
                } else if (model.Activities.length > 0) {
                   var activityNames = new Array();

                    for (var i = 0; i < model.Activities.length; i++) {
                        var activity = model.Activities[i];

                        if (plasmoid.configuration.showOnlyCurrentActivity) {
                            if (activity != activityInfo.currentActivity) {
                                activityNames.push(activityInfo.activityName(model.Activities[i]));
                            }
                        } else if (activity != activityInfo.currentActivity) {
                            activityNames.push(activityInfo.activityName(model.Activities[i]));
                        }
                    }

                    if (plasmoid.configuration.showOnlyCurrentActivity) {
                        if (activityNames.length > 0) {
                            subTextEntries.push(i18nc("Activities a window is currently on (apart from the current one)",
                                "Also available on %1", activityNames.join(", ")));
                        }
                    } else if (activityNames.length > 0) {
                        subTextEntries.push(i18nc("Which activities a window is currently on",
                            "Available on %1", activityNames.join(", ")));
                    }
                }

                return subTextEntries.join("\n");
            }
        }
    }

    Loader {
        anchors.fill: frame
        asynchronous: true
        source: "TaskProgressOverlay.qml"
        active: plasmoid.configuration.smartLaunchersEnabled && task.smartLauncherItem && task.smartLauncherItem.progressVisible
    }

    Item {
        id: iconBox

        anchors {
            left: parent.left
            leftMargin: adjustMargin(true, parent.width, taskFrame.margins.left)
            top: parent.top
            topMargin: adjustMargin(false, parent.height, taskFrame.margins.top)
        }

        width: (label.visible ? height
            : parent.width - adjustMargin(true, parent.width, taskFrame.margins.left)
            - adjustMargin(true, parent.width, taskFrame.margins.right))
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

            active: task.highlighted || (task.contextMenu && task.contextMenu.status == PlasmaComponents.DialogStatus.Open)
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
            active: plasmoid.configuration.smartLaunchersEnabled && height >= units.iconSizes.small
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

    PlasmaComponents.Label {
        id: label

        visible: (inPopup || !iconsOnly && model.IsLauncher !== true
            && (parent.width - iconBox.height - units.smallSpacing) >= (theme.mSize(theme.defaultFont).width * 7))

        anchors {
            fill: parent
            leftMargin: taskFrame.margins.left + iconBox.width + units.smallSpacing
            topMargin: taskFrame.margins.top
            rightMargin: taskFrame.margins.right
            bottomMargin: taskFrame.margins.bottom
        }

        text: model.display
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.PlainText
        verticalAlignment: Text.AlignVCenter
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
            when: task.highlighted || (contextMenu.status == PlasmaComponents.DialogStatus.Open && contextMenu.visualParent == task)

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
            when: model.IsMinimized === true && !(groupDialog.visible && groupDialog.visualParent == task)

            PropertyChanges {
                target: frame
                basePrefix: "minimized"
            }
        },
        State {
            name: "active"
            when: model.IsActive === true || groupDialog.visible && groupDialog.visualParent == task

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
    }
}
