/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.10

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.20 as Kirigami

import org.kde.taskmanager 0.1 as TaskManager

PlasmoidItem {
    Plasmoid.constraintHints: Plasmoid.CanFillArea
    compactRepresentation: windowListButton
    fullRepresentation: windowListView
    switchWidth: Kirigami.Units.gridUnit * 8
    switchHeight: Kirigami.Units.gridUnit * 6

    Component {
        id: windowListView

        ListView {
            clip: true
            Layout.preferredWidth: Kirigami.Units.gridUnit * 10
            Layout.preferredHeight: Kirigami.Units.gridUnit * 12
            model: TaskManager.TasksModel {
                id: tasksModel

                sortMode: TaskManager.TasksModel.SortVirtualDesktop
                groupMode: TaskManager.TasksModel.GroupDisabled
            }
            delegate: PlasmaComponents.ItemDelegate {

                width: ListView.view.width

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        id: iconItem

                        source: model.decoration
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
                        text: model.display
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                    }
                }

                onClicked: tasksModel.requestActivate(tasksModel.makeModelIndex(model.index))
            }
        }
    }

    // Only exists because the default CompactRepresentation doesn't expose the
    // ability to show text below or beside the icon.
    // TODO remove once it gains that feature.
    Component {
        id: windowListButton

        MenuButton {
            Layout.minimumWidth: implicitWidth
            Layout.maximumWidth: implicitWidth
            Layout.fillHeight: Plasmoid.formFactor === PlasmaCore.Types.Horizontal
            Layout.fillWidth: Plasmoid.formFactor === PlasmaCore.Types.Vertical

            onClicked: tasksMenu.open()
            down: pressed || tasksMenu.status === PlasmaExtras.Menu.Open

            Accessible.name: Plasmoid.title
            Accessible.description: toolTipSubText

            text: if (tasksModel.activeTask.valid) {
                return tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.AppName) ||
                       tasksModel.data(tasksModel.activeTask, 0 /* display name, window title if app name not present */)
            } else {
                return i18n("Plasma Desktop")
            }

            iconSource: if (tasksModel.activeTask.valid) {
                return tasksModel.data(tasksModel.activeTask, 1 /* decorationrole */)
            } else {
                return "start-here-kde-symbolic"
            }

            TaskManager.TasksModel {
                id: tasksModel

                sortMode: TaskManager.TasksModel.SortVirtualDesktop
                groupMode: TaskManager.TasksModel.GroupDisabled
            }

            PlasmaExtras.ModelContextMenu {
                id: tasksMenu

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

                model: tasksModel
                onClicked: (model) =>
                    tasksModel.requestActivate(tasksModel.makeModelIndex(model.index))
            }
        }
    }
}
