/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami 2 as Kirigami
import "code/tools.js" as TaskTools

Flow {
    id: taskGrid

    layoutDirection: (plasmoid.configuration.reverseMode && !tasks.vertical)
        ? (Qt.application.layoutDirection === Qt.LeftToRight)
            ? Qt.RightToLeft
            : Qt.LeftToRight
        : Qt.application.layoutDirection

    readonly property alias repeater: repeater
    readonly property int effectiveLauncherCount: {
        let count = tasksModel.launcherCount;
        for (let i = 0; i < repeater.count; ++i) {
            const item = repeater.itemAt(i);
            if (item && item.m.IsStartup) {
                count -= 1;
            }
        }
        return count;
    }
    readonly property int logicalTaskCount: repeater.count - effectiveLauncherCount

    readonly property real preferredMinWidth: tasks.preferredMaxWidth - (tasks.vertical ? verticalMargins : horizontalMargins)
    readonly property real preferredMinHeight: Kirigami.Units.iconSizes.sizeForLabels + 4 // TODO FIXME UPSTREAM: Port to proper font metrics for descenders once we have access to them.
    readonly property real preferredMaxHeight: {
        if (tasks.vertical) {
            return tasks.verticalMargins +
                Math.min(
                    // Do not allow the preferred icon size to exceed the width of
                    // the vertical task manager.
                    tasks.width,
                    tasks.iconsOnly ? tasks.width :
                        Math.max(
                            Kirigami.Units.iconSizes.sizeForLabels,
                            Kirigami.Units.iconSizes.medium
                        )
                );
        } else {
            return tasks.verticalMargins +
                    Math.min(
                        Kirigami.Units.iconSizes.small * 3,
                        Kirigami.Units.iconSizes.sizeForLabels * 3
                    );
        }
    }
    readonly property int maxStripes: {
        const length = tasks.vertical ? tasks.width : tasks.height;
        const minimum = tasks.vertical ? tasks.preferredMaxWidth : taskGrid.preferredMinHeight;
        return Math.min(plasmoid.configuration.maxStripes, Math.max(1, Math.floor(length / minimum)));
    }
    readonly property int tasksPerStripe: {
        if (plasmoid.configuration.forceStripes) {
            return Math.ceil(repeater.count / maxStripes);
        } else {
            const length = tasks.vertical ? tasks.height : tasks.width;
            const minimum = tasks.vertical ? preferredMinHeight : preferredMinWidth;
            return Math.floor(length / minimum);
        }
    }
    readonly property int currentStripes: {
        const stripes = plasmoid.configuration.forceStripes ? plasmoid.configuration.maxStripes : Math.min(plasmoid.configuration.maxStripes, Math.ceil(repeater.count / tasksPerStripe));
        return Math.min(stripes, maxStripes);
    }

    readonly property real taskWidth: {
        if (tasks.vertical) {
            return Math.floor(tasks.width / currentStripes);
        } else {
            const leftLength = tasks.width - (effectiveLauncherCount % tasksPerStripe) * Math.min(taskGrid.taskHeight, tasks.iconsOnlyTaskLength);
            if (maxStripes == currentStripes && Math.max(1, repeater.count) > tasksPerStripe) {
                return Math.floor(leftLength / Math.ceil(logicalTaskCount / maxStripes));
            } else {
                return Math.min(preferredMaxWidth, Math.floor(leftLength / Math.min(logicalTaskCount, tasksPerStripe)));
            }
        }
    }
    readonly property real taskHeight: {
        if (tasks.vertical) {
            if (maxStripes == currentStripes && Math.max(1, repeater.count) > tasksPerStripe) {
                return Math.floor(tasks.height / Math.ceil(repeater.count / maxStripes));
            } else {
                return Math.min(preferredMaxHeight, Math.floor(tasks.height / Math.min(repeater.count, tasksPerStripe)));
            }
        } else {
            return Math.floor(tasks.height / currentStripes);
        }
    }

    property bool animating: false

    move: Transition {
        SequentialAnimation {
            PropertyAction {
                target: taskGrid
                property: "animating"
                value: true
            }

            NumberAnimation {
                properties: "x, y"
                easing.type: Easing.OutQuad
                duration: Kirigami.Units.longDuration
            }

            PropertyAction {
                target: taskGrid
                property: "animating"
                value: false
            }

            ScriptAction {
                script: {
                    publishIconGeometries();
                }
            }
        }
    }

    function publishIconGeometries() {
        if (TaskTools.taskManagerInstanceCount >= 2) {
            return;
        }
        for (let i = 0; i < repeater.count - 1; ++i) {
            const task = repeater.itemAt(i);

            if (task && !task.m.IsLauncher && !task.m.IsStartup) {
                tasks.tasksModel.requestPublishDelegateGeometry(tasks.tasksModel.makeModelIndex(task.itemIndex),
                    backend.globalRect(task), task);
            }
        }
    }

    Repeater {
        id: repeater

        model: tasksModel
        delegate: Task {
            width: model.IsLauncher && !tasks.vertical ? Math.min(taskGrid.taskHeight, tasks.iconsOnlyTaskLength) : taskGrid.taskWidth
            height: taskGrid.taskHeight
        }
    }
}