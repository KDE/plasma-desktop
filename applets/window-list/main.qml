/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami

import org.kde.taskmanager as TaskManager

PlasmoidItem {
    id: root

    property string lastActiveTaskName: ""
    property /*QIcon*/ var lastActiveTaskIcon: ""

    Plasmoid.constraintHints: Plasmoid.CanFillArea
    compactRepresentation: windowListButton
    fullRepresentation: windowList
    switchWidth: Kirigami.Units.gridUnit * 8
    switchHeight: Kirigami.Units.gridUnit * 6

    TaskManager.VirtualDesktopInfo {
        id: virtualDesktopInfo
    }

    TaskManager.ActivityInfo {
        id: activityInfo
        readonly property string nullUuid: "00000000-0000-0000-0000-000000000000"
    }

    TaskManager.TasksModel {
        id: tasksModel

        virtualDesktop: virtualDesktopInfo.currentDesktop
        screenGeometry: Plasmoid.containment.screenGeometry
        activity: activityInfo.currentActivity

        sortMode: Plasmoid.configuration.sortingStrategy
        groupMode: TaskManager.TasksModel.GroupDisabled

        filterByVirtualDesktop: Plasmoid.configuration.showOnlyCurrentDesktop
        filterByScreen: Plasmoid.configuration.showOnlyCurrentScreen
        filterByActivity: Plasmoid.configuration.showOnlyCurrentActivity
        filterNotMinimized: Plasmoid.configuration.showOnlyMinimized
    }

    Component {
        id: windowList

        ListView {
            id: windowListView

            clip: true
            Layout.preferredWidth: Kirigami.Units.gridUnit * 10
            Layout.preferredHeight: Kirigami.Units.gridUnit * 12
            model: tasksModel

            Connections {
                target: root
                function onExpandedChanged(expanded) {
                    if (expanded) {
                        windowListView.currentIndex = -1

                        // Needed for when for expanded with Global Shortcut
                        if (tasksModel.activeTask.valid) {
                            root.lastActiveTaskName = tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.AppName) ||
                            tasksModel.data(tasksModel.activeTask, 0 /* display name, window title if app name not present */)
                            root.lastActiveTaskIcon = tasksModel.data(tasksModel.activeTask, 1 /* decorationrole */)
                        } else {
                            root.lastActiveTaskName = ""
                            root.lastActiveTaskIcon = ""
                        }
                    }
                }
            }

            // focus is needed to receive key events on desktop containment works in panel without this.
            focus: true
            keyNavigationWraps: true

            highlight: PlasmaExtras.Highlight {
                visible: windowListView.currentItem
                active: windowListView.focus
                pressed: windowListView.currentItem && windowListView.currentItem.pressed
            }

            highlightMoveDuration: 0
            highlightResizeDuration: 0

            Keys.onEnterPressed: {
                if (currentIndex >= 0 && currentIndex < windowListView.count) {
                    tasksModel.requestActivate(tasksModel.makeModelIndex(currentIndex));
                }
            }

            Keys.onReturnPressed: {
                if (currentIndex >= 0 && currentIndex < windowListView.count) {
                    tasksModel.requestActivate(tasksModel.makeModelIndex(currentIndex));
                }
            }

            Keys.onTabPressed: {
                incrementCurrentIndex();
            }

            Keys.onBacktabPressed: {
                decrementCurrentIndex();
            }

            section {
                property: switch (Plasmoid.configuration.sortingStrategy) {
                    case TaskManager.TasksModel.SortVirtualDesktop:
                        return "VirtualDesktops"; // AbstractTasksModel::AdditionalRoles::VirtualDesktops
                    case TaskManager.TasksModel.SortActivity:
                        return "Activities";  // AbstractTasksModel::AdditionalRoles::Activities
                    default:
                        return "";
                }
                delegate: Kirigami.ListSectionHeader {
                    required property var section
                    width: windowListView.width
                    text: {
                        switch (Plasmoid.configuration.sortingStrategy) {
                        case TaskManager.TasksModel.SortVirtualDesktop:
                            // Section contains the virtual desktop id. In case the application is on multiple desktops, it is empty.
                            return section ? virtualDesktopInfo.desktopNames[virtualDesktopInfo.desktopIds.indexOf(section)] : "";
                        case TaskManager.TasksModel.SortActivity:
                            // Section contains the activity id. In case the application is on multiple activities, it is empty.
                            return section ? activityInfo.activityName(section) : "";
                        }
                        return "";
                    }
                }
            }

            delegate: PlasmaComponents.ItemDelegate {
                id: delegate

                required property var model
                required property var decoration


                width: ListView.view.width

                highlighted: ListView.isCurrentItem

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        id: iconItem

                        source: delegate.decoration
                        visible: source !== "" && iconItem.valid

                        implicitWidth: Kirigami.Units.iconSizes.sizeForLabels
                        implicitHeight: Kirigami.Units.iconSizes.sizeForLabels
                    }
                    // Fall back to a generic icon if the application doesn't provide a valid one
                    Kirigami.Icon {
                        source: "preferences-system-windows"
                        visible: !iconItem.valid

                        implicitWidth: Kirigami.Units.iconSizes.sizeForLabels
                        implicitHeight: Kirigami.Units.iconSizes.sizeForLabels
                    }
                    PlasmaComponents.Label {
                        Layout.fillWidth: true
                        text: delegate.model.display
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                    }
                }

                onHoveredChanged: {
                    if (hovered) {
                        windowListView.currentIndex = model.index
                    }
                }

                onClicked: {
                    windowListView.currentIndex = model.index
                    tasksModel.requestActivate(tasksModel.makeModelIndex(model.index))
                }

            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 2)
                visible: windowListView.count === 0
                icon.source: "edit-none"
                text: i18nc("@info:placeholder", "No open windows")
            }
        }
    }

    // Only exists because the default CompactRepresentation doesn't expose the
    // ability to show text below or beside the icon.
    // TODO remove once it gains that feature.
    Component {
        id: windowListButton

        MenuButton {
            id: menuButton

            Layout.minimumWidth: implicitWidth
            Layout.maximumWidth: implicitWidth
            Layout.fillHeight: Plasmoid.formFactor === PlasmaCore.Types.Horizontal
            Layout.fillWidth: Plasmoid.formFactor === PlasmaCore.Types.Vertical

            onClicked: {
                if (tasksModel.activeTask.valid) {
                    root.lastActiveTaskName = tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.AppName) ||
                       tasksModel.data(tasksModel.activeTask, 0 /* display name, window title if app name not present */)
                    root.lastActiveTaskIcon = tasksModel.data(tasksModel.activeTask, 1 /* decorationrole */)
                }
                root.expanded = !root.expanded
            }
            down: pressed || root.expanded

            Accessible.name: Plasmoid.title
            Accessible.description: root.toolTipSubText

            text: if (root.expanded && root.lastActiveTaskName !== "") {
                return root.lastActiveTaskName
            } else if (tasksModel.activeTask.valid) {
                return tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.AppName) ||
                       tasksModel.data(tasksModel.activeTask, 0 /* display name, window title if app name not present */)
            } else {
                return i18nc("@title:window title shown e.g. for desktop and expanded widgets", "Plasma Desktop")
            }

            iconSource: if (expanded && root.lastActiveTaskIcon) {
                return root.lastActiveTaskIcon
            } else if (tasksModel.activeTask.valid) {
                return tasksModel.data(tasksModel.activeTask, 1 /* decorationrole */)
            } else {
                return "start-here-kde-symbolic"
            }

            Timer {
                id: hoverOpenTimer
                interval: Plasmoid?.configuration?.hoverOpenDelay ?? 300
                repeat: false
                onTriggered: {
                    root.expanded = true
                }
            }

            onHoveredChanged: {
                if (hovered) {
                    if (tasksModel.activeTask.valid) {
                        root.lastActiveTaskName = tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.AppName) ||
                       tasksModel.data(tasksModel.activeTask, 0 /* display name, window title if app name not present */)
                       root.lastActiveTaskIcon = tasksModel.data(tasksModel.activeTask, 1 /* decorationrole */)
                    }
                    if (Plasmoid.configuration.openOnHover) {
                        hoverOpenTimer.start()
                    }
                } else {
                    hoverOpenTimer.stop()
                }
            }
        }
    }
}
