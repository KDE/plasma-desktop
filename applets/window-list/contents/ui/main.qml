/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.10

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.20 as Kirigami

import org.kde.taskmanager 0.1 as TaskManager

PlasmoidItem {
    Plasmoid.constraintHints: PlasmaCore.Types.CanFillArea
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
            delegate: Kirigami.BasicListItem {
                id: del
                onClicked: tasksModel.requestActivate(tasksModel.makeModelIndex(model.index))

                leading: Item {
                    width: iconItem.width

                    PlasmaCore.IconItem {
                        id: iconItem

                        source: model.decoration
                        visible: source !== "" && iconItem.valid

                        implicitWidth: parent.height
                        implicitHeight: parent.height
                    }
                    // Fall back to a generic icon if the application doesn't provide a valid one
                    PlasmaCore.IconItem {
                        source: "preferences-system-windows"
                        visible: !iconItem.valid

                        implicitWidth: parent.height
                        implicitHeight: parent.height
                    }
                }
                text: model.display
            }
        }
    }
    Component {
        id: windowListButton

        MenuButton {
            Layout.minimumWidth: implicitWidth
            Layout.maximumWidth: implicitWidth
            Layout.fillHeight: plasmoid.formFactor === PlasmaCore.Types.Horizontal
            Layout.fillWidth: plasmoid.formFactor === PlasmaCore.Types.Vertical

            onClicked: tasksMenu.open()
            down: pressed || tasksMenu.status === PlasmaExtras.DialogStatus.Open

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
                return "start-here-kde"
            }

            TaskManager.TasksModel {
                id: tasksModel

                sortMode: TaskManager.TasksModel.SortVirtualDesktop
                groupMode: TaskManager.TasksModel.GroupDisabled
            }

            PlasmaExtras.ModelContextMenu {
                id: tasksMenu

                model: tasksModel
                onClicked: (model) =>
                    tasksModel.requestActivate(tasksModel.makeModelIndex(model.index))
            }
        }
    }
}
