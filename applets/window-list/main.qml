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
import org.kde.kitemmodels as KItemModels

import org.kde.taskmanager as TaskManager

PlasmoidItem {
    id: root

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

        sortMode: sortModeEnumValue(Plasmoid.configuration.sortingStrategy)
        groupMode: TaskManager.TasksModel.GroupDisabled

        filterByVirtualDesktop: Plasmoid.configuration.showOnlyCurrentDesktop
        filterByScreen: Plasmoid.configuration.showOnlyCurrentScreen
        filterByActivity: Plasmoid.configuration.showOnlyCurrentActivity
        filterNotMinimized: Plasmoid.configuration.showOnlyMinimized

        function sortModeEnumValue(index: int): /*TaskManager.TasksModel.SortMode*/ int {
            switch (index) {
            case 0:
                return TaskManager.TasksModel.SortDisabled;
            case 1: // Unused
                return TaskManager.TasksModel.SortManual; 
            case 2:
                return TaskManager.TasksModel.SortAlpha;
            case 3:
                return TaskManager.TasksModel.SortVirtualDesktop;
            case 4:
                return TaskManager.TasksModel.SortActivity;
            case 5:
                return TaskManager.TasksModel.SortLastActivated;
            case 6:
                return TaskManager.TasksModel.SortWindowPositionHorizontal;
            default:
                return TaskManager.TasksModel.SortDisabled;
            }
        }
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
                    case 3:
                        return "VirtualDesktops"; // AbstractTasksModel::AdditionalRoles::VirtualDesktops
                    case 4:
                        return "Activities";  // AbstractTasksModel::AdditionalRoles::Activities
                    default:
                        return "";
                }
                delegate: Kirigami.ListSectionHeader {
                    required property var section
                    width: windowListView.width
                    text: {
                        switch (Plasmoid.configuration.sortingStrategy) {
                        case 3:
                            if (section) {
                                const idx = virtualDesktopInfo.desktopIds.indexOf(section);
                                return idx !== -1 ? virtualDesktopInfo.desktopNames[idx] : i18n("Desktop %1", section);
                            }
                            break;
                        case 4:
                            if (section && section !== activityInfo.nullUuid) {
                                const activityName = activityInfo.activityName(section);
                                return activityName || section;
                            }
                            break;
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

            onClicked: tasksMenu.openRelative()
            down: pressed || tasksMenu.status === PlasmaExtras.Menu.Open

            Accessible.name: Plasmoid.title
            Accessible.description: root.toolTipSubText

            text: if (tasksModel.activeTask.valid) {
                return tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.AppName) ||
                       tasksModel.data(tasksModel.activeTask, 0 /* display name, window title if app name not present */)
            } else {
                return i18nc("@title:window title shown e.g. for desktop and expanded widgets", "Plasma Desktop")
            }

            iconSource: if (tasksModel.activeTask.valid) {
                return tasksModel.data(tasksModel.activeTask, 1 /* decorationrole */)
            } else {
                return "start-here-kde-symbolic"
            }

            PlasmaExtras.ModelContextMenu {
                id: tasksMenu
                visualParent: menuButton

                placement: {
                   if (Plasmoid.location === PlasmaCore.Types.LeftEdge) {
                       return PlasmaExtras.Menu.RightPosedTopAlignedPopup
                   } else if (Plasmoid.location === PlasmaCore.Types.TopEdge) {
                       return PlasmaExtras.Menu.BottomPosedLeftAlignedPopup
                   } else if (Plasmoid.location === PlasmaCore.Types.RightEdge) {
                       return PlasmaExtras.Menu.LeftPosedTopAlignedPopup
                   } else {
                       return PlasmaExtras.Menu.TopPosedLeftAlignedPopup
                   }
                }

                property ListModel noWindowModel: ListModel {
                    ListElement {
                        display: "" // filled by Component.onCompleted
                        decoration: "edit-none"
                    }
                    Component.onCompleted: tasksMenu.noWindowModel.setProperty(0, "display", i18nc("@info:placeholder", "No open windows"))
                }

                model: tasksModel.count === 0 ? noWindowModel : tasksModel
                onClicked: (model) => {
                    if (tasksModel.count > 0) {
                        tasksModel.requestActivate(tasksModel.makeModelIndex(model.index));
                    }
                }
            }

            Timer {
                id: hoverOpenTimer
                interval: Plasmoid?.configuration?.hoverOpenDelay ?? 300
                repeat: false
                onTriggered: {
                    if (tasksMenu?.status === PlasmaExtras.Menu.Closed) {
                        tasksMenu.openRelative()
                    }
                }
            }

            onHoveredChanged: {
                if (hovered && Plasmoid.configuration.openOnHover) {
                    hoverOpenTimer.start()
                } else {
                    hoverOpenTimer.stop()
                }
            }
        }
    }
}
