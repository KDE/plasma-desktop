/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.10

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PC2
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.12 as Kirigami

import org.kde.taskmanager 0.1 as TaskManager

Item {
    Plasmoid.constraintHints: PlasmaCore.Types.CanFillArea
    Plasmoid.compactRepresentation: windowListButton
    Plasmoid.fullRepresentation: windowListView
    Plasmoid.switchWidth: PlasmaCore.Units.gridUnit * 8
    Plasmoid.switchHeight: PlasmaCore.Units.gridUnit * 6

    Component {
        id: windowListView

        ListView {
            clip: true
            Layout.preferredWidth: PlasmaCore.Units.gridUnit * 10
            Layout.preferredHeight: PlasmaCore.Units.gridUnit * 12
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
                        visible: source !== ""

                        anchors.top: parent.top
                        anchors.verticalCenter: parent.verticalCenter

                        implicitWidth: PlasmaCore.Units.roundToIconSize(parent.height)
                        implicitHeight: PlasmaCore.Units.roundToIconSize(parent.height)
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
            down: pressed || tasksMenu.status === PC2.DialogStatus.Open

            text: if (tasksModel.activeTask.valid) {
                return tasksModel.data(tasksModel.activeTask, TaskManager.AbstractTasksModel.AppName) ||
                       tasksModel.data(tasksModel.activeTask, 0 /* display name, window title if app name not present */)
            } else return i18n("Plasma Desktop")
            iconSource: if (tasksModel.activeTask.valid) {
                return tasksModel.data(tasksModel.activeTask, 1 /* decorationrole */)
            } else return ""

            TaskManager.TasksModel {
                id: tasksModel

                sortMode: TaskManager.TasksModel.SortVirtualDesktop
                groupMode: TaskManager.TasksModel.GroupDisabled
            }

            PC2.ModelContextMenu {
                id: tasksMenu

                model: tasksModel
                onClicked: (model) =>
                    tasksModel.requestActivate(tasksModel.makeModelIndex(model.index))
            }
        }
    }
}
