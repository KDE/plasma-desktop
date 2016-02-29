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

import org.kde.plasma.private.taskmanager 0.1 as TaskManager

import "../code/layout.js" as LayoutManager
import "../code/tools.js" as TaskTools

MouseArea {
    id: task

    width: groupDialog.mainItem.width
    height: units.iconSizes.small + LayoutManager.verticalMargins()

    visible: false

    LayoutMirroring.enabled: (Qt.application.layoutDirection == Qt.RightToLeft)
    LayoutMirroring.childrenInherit: (Qt.application.layoutDirection == Qt.RightToLeft)

    property int itemIndex: index
    property int itemId: model.Id
    property bool inPopup: false
    property bool isGroupParent: model.hasModelChildren
    property bool isLauncher: model.IsLauncher
    property bool isStartup: model.IsStartup
    property bool demandsAttention: model.DemandsAttention
    property bool showingContextMenu: false
    property int textWidth: label.implicitWidth
    property bool pressed: false
    property int pressX: -1
    property int pressY: -1
    property Item busyIndicator
    property int wheelDelta: 0

    readonly property bool smartLauncherEnabled: plasmoid.configuration.smartLaunchersEnabled && !inPopup && !isStartup
    property QtObject smartLauncherItem: null

    acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MidButton
    hoverEnabled: true

    onItemIndexChanged: {
        if (!inPopup && !tasks.vertical && LayoutManager.calculateStripes() > 1) {
            var newWidth = LayoutManager.taskWidth();

            if (index == backend.tasksModel.launcherCount) {
                newWidth += LayoutManager.launcherLayoutWidthDiff();
            }

            width = newWidth;
        }
    }

    onIsStartupChanged: {
        if (!isStartup) {
            tasks.itemGeometryChanged(task, itemId);
            busyIndicator.visible = false;
            busyIndicator.running = false;
            icon.visible = true;
        }
    }

    onDemandsAttentionChanged: {
        tasks.updateStatus(demandsAttention);
    }

    onContainsMouseChanged:  {
        if (!containsMouse) {
            pressed = false;
        }

        tasks.itemHovered(model.Id, containsMouse);
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

            showingContextMenu = true;
            tasks.itemContextMenu(task, plasmoid.action("configure"));
        }
    }

    onReleased: {
        if (pressed) {
            if (mouse.button == Qt.MidButton) {
                if (plasmoid.configuration.middleClickAction == TaskManager.Backend.NewInstance) {
                    tasks.launchNewInstance(model.Id);
                } else if (plasmoid.configuration.middleClickAction == TaskManager.Backend.Close) {
                    tasks.closeByItemId(model.Id);
                }
            } else if (mouse.button == Qt.LeftButton) {
                if (mouse.modifiers & Qt.ShiftModifier) {
                    tasks.launchNewInstance(model.Id);
                } else if (isGroupParent) {
                    if ((iconsOnly || mouse.modifiers == Qt.ControlModifier) && backend.canPresentWindows()) {
                        tasks.presentWindows(model.Id);
                    } else if (groupDialog.visible) {
                        groupDialog.visible = false;
                    } else {
                        groupDialog.visualParent = task;
                        groupDialog.visible = true;
                    }
                } else {
                    tasks.activateItem(model.Id, true);
                }
            }
        }

        pressed = false;
        pressX = -1;
        pressY = -1;
    }

    onPositionChanged: {
        if (pressX != -1 && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
            tasks.dragSource = task;
            dragHelper.startDrag(task, model.MimeType, model.MimeData,
                model.LauncherUrl, model.DecorationRole);
            pressX = -1;
            pressY = -1;

            return;
        }
    }

    onWheel: {
        if (plasmoid.configuration.wheelEnabled) {
            wheelDelta = TaskTools.wheelActivateNextPrevTask(task, wheelDelta, wheel.angleDelta.y);
        }
    }

    onSmartLauncherEnabledChanged: {
        if (smartLauncherEnabled && !smartLauncherItem) {
            var smartLauncher = Qt.createQmlObject("
    import org.kde.plasma.private.taskmanager 0.1 as TaskManager;
    TaskManager.SmartLauncherItem { }", task);

            smartLauncher.launcherUrl = Qt.binding(function() { return model.LauncherUrl; });

            smartLauncherItem = smartLauncher;
        }
    }

    Connections {
        target: backend

        onShowingContextMenuChanged: {
            if (task.showingContextMenu && !backend.showingContextMenu) {
                task.showingContextMenu = false;
            }
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
        prefix: TaskTools.taskPrefix("normal")
        onRepaintNeeded: prefix = TaskTools.taskPrefix(basePrefix);
        onBasePrefixChanged: prefix = TaskTools.taskPrefix(basePrefix);

        PlasmaCore.ToolTipArea {
            id: toolTip

            anchors.fill: parent

            active: !inPopup && !groupDialog.visible && plasmoid.configuration.showToolTips
            interactive: true
            location: plasmoid.location

            mainItem: toolTipDelegate

            //FIXME TODO: highlightWindows: plasmoid.configuration.highlightWindows
            onContainsMouseChanged:  {
                if (containsMouse) {
                    toolTipDelegate.windows = model.WindowList;
                    toolTipDelegate.mainText = model.DisplayRole;
                    toolTipDelegate.icon = model.DecorationRole;
                    toolTipDelegate.subText = model.IsLauncher ? model.GenericName
                        : toolTip.generateSubText(model);
                    toolTipDelegate.launcherUrl = model.LauncherUrl;
                }
            }

            function generateSubText(task) {
                var subTextEntries = new Array();

                if (!plasmoid.configuration.showOnlyCurrentDesktop && backend.numberOfDesktops() > 1) {
                    subTextEntries.push(i18n("On %1", task.DesktopName));
                }

                if (task.OnAllActivities && backend.numberOfActivities() > 1) {
                    subTextEntries.push(i18nc("Which virtual desktop a window is currently on",
                        "Available on all activities"));
                } else if (plasmoid.configuration.showOnlyCurrentActivity) {
                    if (task.OtherActivityNames.length > 0) {
                        subTextEntries.push(i18nc("Activities a window is currently on (apart from the current one)",
                                                "Also available on %1",
                                                task.OtherActivityNames.join(", ")));
                    }
                } else if (task.ActivityNames.length > 0) {
                    subTextEntries.push(i18nc("Which activities a window is currently on",
                                            "Available on %1",
                                            task.ActivityNames.join(", ")));
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
            leftMargin: tasks.vertical && !label.visible ? adjustMargin(true, parent.width, taskFrame.margins.left) : taskFrame.margins.left
            top: parent.top
            topMargin: adjustMargin(false, parent.height, taskFrame.margins.top);
            bottom: parent.bottom
            bottomMargin: adjustMargin(false, parent.height, taskFrame.margins.bottom);
            right: (tasks.vertical && !label.visible && !inPopup) ? parent.right : undefined
            rightMargin: tasks.vertical && !label.visible ? adjustMargin(true, parent.width, taskFrame.margins.right) : undefined
        }

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

        width: inPopup ? units.iconSizes.small : Math.min(height, parent.width - LayoutManager.horizontalMargins())

        PlasmaCore.IconItem {
            id: icon

            anchors.fill: parent

            visible: false

            active: task.containsMouse || task.showingContextMenu
            enabled: true
            usesPlasmaTheme: false

            source: model.DecorationRole

            onVisibleChanged: {
                if (visible && busyIndicator) {
                    busyIndicator.destroy();
                }
            }
        }

        Loader {
            anchors.fill: icon
            asynchronous: true
            source: "TaskBadgeOverlay.qml"
            active: plasmoid.configuration.smartLaunchersEnabled && height >= units.iconSizes.small
                    && icon.visible && task.smartLauncherItem && task.smartLauncherItem.countVisible
        }

        PlasmaComponents.BusyIndicator {
            id: busyIndicator

            anchors.fill: parent

            visible: false

            running: false
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
    }

    TaskManager.TextLabel {
        id: label

        anchors {
            fill: parent
            leftMargin: taskFrame.margins.left + iconBox.width + units.smallSpacing
            topMargin: taskFrame.margins.top
            rightMargin: taskFrame.margins.right
            bottomMargin: taskFrame.margins.bottom
        }

        visible: (inPopup || !iconsOnly && !model.IsLauncher && (parent.width - LayoutManager.horizontalMargins()) >= (theme.mSize(theme.defaultFont).width * 7))

        enabled: true

        text: (!inPopup && iconsOnly) ? "" : model.DisplayRole
        color: theme.textColor
        elide: !inPopup
    }

    states: [
        State {
            name: "launcher"
            when: model.IsLauncher

            PropertyChanges {
                target: frame
                basePrefix: ""
            }
        },
        State {
            name: "hovered"
            when: containsMouse || showingContextMenu

            PropertyChanges {
                target: frame
                basePrefix: "hover"
            }
        },
        State {
            name: "attention"
            when: model.DemandsAttention || (task.smartLauncherItem && task.smartLauncherItem.urgent)

            PropertyChanges {
                target: frame
                basePrefix: "attention"
            }
        },
        State {
            name: "minimized"
            when: model.Minimized && !(groupDialog.visible && groupDialog.target == task)

            PropertyChanges {
                target: frame
                basePrefix: "minimized"
            }
        },
        State {
            name: "active"
            when: model.Active || groupDialog.visible && groupDialog.target == task

            PropertyChanges {
                target: frame
                basePrefix: "focus"
            }
        }
    ]

    Component.onCompleted: {
        if (model.IsStartup) {
            busyIndicator.running = true;
            busyIndicator.visible = true;
        } else {
            icon.visible = true;
        }

        if (model.hasModelChildren) {
            var component = Qt.createComponent("GroupExpanderOverlay.qml");
            component.createObject(task);
        }
    }
/*
    Component.onDestruction: {
        if (groupDialog.visible && groupDialog.groupItemId == model.Id) {
            groupDialog.visible = false;
        }
    }
*/
}
