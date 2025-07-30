/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.10

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.20 as Kirigami

import org.kde.taskmanager 0.1 as TaskManager

PlasmoidItem {
    id: root

    Plasmoid.constraintHints: Plasmoid.CanFillArea
    compactRepresentation: windowListButton
    fullRepresentation: windowList
    switchWidth: Kirigami.Units.gridUnit * 8
    switchHeight: Kirigami.Units.gridUnit * 6

    TaskManager.TasksModel {
        id: tasksModel

        sortMode: TaskManager.TasksModel.SortVirtualDesktop
        groupMode: TaskManager.TasksModel.GroupDisabled
    }

    Component {
        id: windowList

        ListView {
            id: windowListView

            clip: true
            Layout.preferredWidth: Kirigami.Units.gridUnit * 10
            Layout.preferredHeight: Kirigami.Units.gridUnit * 12
            model: tasksModel

            delegate: PlasmaComponents.ItemDelegate {
                id: delegate

                required property var model
                required property var decoration


                width: ListView.view.width

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

                onClicked: tasksModel.requestActivate(tasksModel.makeModelIndex(model.index))

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
            Layout.minimumWidth: implicitWidth
            Layout.maximumWidth: implicitWidth
            Layout.fillHeight: Plasmoid.formFactor === PlasmaCore.Types.Horizontal
            Layout.fillWidth: Plasmoid.formFactor === PlasmaCore.Types.Vertical

            onClicked: tasksMenu.open()
            down: pressed || tasksMenu.status === PlasmaExtras.Menu.Open

            Accessible.name: Plasmoid.title
            Accessible.description: root.toolTipSubText

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

                property ListModel noWindowModel: ListModel {
                    ListElement {
                        display: "No open windows"
                        decoration: "edit-none"
                    }
                }

                model: tasksModel.count === 0 ? noWindowModel : tasksModel
                onClicked: (model) => {
                    if (tasksModel.count > 0) {
                        tasksModel.requestActivate(tasksModel.makeModelIndex(model.index));
                    }
                }
            }
        }
    }
}
