/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2 as Kirigami
import "code/tools.js" as TaskTools

ListView {
    id: taskList

    property bool animating: false

    boundsBehavior: Flickable.StopAtBounds
    clip: true
    delegate: Task {
        width: tasks.vertical ? tasks.width : (model.IsLauncher ? tasks.iconsOnlyTaskLength : taskList.taskLength)
        height: tasks.vertical ? tasks.iconsOnlyTaskLength : tasks.height
    }
    model: tasksModel
    layoutDirection: (plasmoid.configuration.reverseMode && !tasks.vertical)
        ? (Qt.application.layoutDirection === Qt.LeftToRight)
            ? Qt.RightToLeft
            : Qt.LeftToRight
        : Qt.application.layoutDirection
    orientation: tasks.vertical ? ListView.Vertical : ListView.Horizontal

    readonly property int effectiveLauncherCount: {
        let count = tasksModel.launcherCount;
        for (let i = 0; i < taskList.count; ++i) {
            const item = taskList.itemAtIndex(i);
            if (item && item.m.IsStartup) {
                count -= 1;
            }
        }
        return count;
    }
    readonly property real taskLength: {
        if (tasks.iconsOnly) {
            return tasks.iconsOnlyTaskLength;
        } else {
            const leftLength = (tasks.vertical ? tasks.height : tasks.width) - tasksModel.launcherCount * iconsOnlyTaskLength;
            const expectedTaskLength = Math.min(tasks.preferredMaxWidth, Math.floor(leftLength / ((taskList.count - taskList.effectiveLauncherCount) || 1)));

            if (tasks.vertical) {
                return Math.max(tasks.iconsOnlyTaskLength, expectedTaskLength);
            }
            return Math.max(tasks.iconsOnlyTaskLength, expectedTaskLength);
        }
    }
    readonly property real totalLength: tasks.iconOnly ? taskList.count * tasks.iconsOnlyTaskLength : (taskList.count - taskList.effectiveLauncherCount) * taskList.taskLength + taskList.effectiveLauncherCount * tasks.iconsOnlyTaskLength

    move: Transition {
        SequentialAnimation {
            PropertyAction {
                target: taskList
                property: "animating"
                value: true
            }

            NumberAnimation {
                properties: "x, y"
                easing.type: Easing.OutQuad
                duration: Kirigami.Units.longDuration
            }

            PropertyAction {
                target: taskList
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
    moveDisplaced: Transition {
        NumberAnimation {
            properties: "x, y"
            easing.type: Easing.OutQuad
            duration: Kirigami.Units.longDuration
        }
    }
    removeDisplaced: moveDisplaced

    function publishIconGeometries() {
        if (TaskTools.taskManagerInstanceCount >= 2) {
            return;
        }
        for (let i = 0; i < count - 1; ++i) {
            const task = itemAtIndex(i);

            if (task && !task.m.IsLauncher && !task.m.IsStartup) {
                tasks.tasksModel.requestPublishDelegateGeometry(tasks.tasksModel.makeModelIndex(task.itemIndex),
                    backend.globalRect(task), task);
            }
        }
    }

    PC3.Button {
        id: moveLeftButton
        anchors {
            top: parent.top
            left: parent.left
        }
        width: tasks.vertical ? tasks.width : tasks.height / 2
        height: tasks.vertical ? tasks.width / 2 : tasks.height
        visible: (tasks.vertical ? taskList.contentY : taskList.contentX) > 0 || pressed
        autoRepeat: true
        icon.name: tasks.vertical ? "arrow-up" : "arrow-left"

        onClicked: taskList.positionViewAtIndex(Math.max(0, taskList.itemAt(tasks.vertical ? tasks.width / 2 : taskList.contentX, tasks.vertical ? taskList.contentY : tasks.height / 2).itemIndex - 1), ListView.Beginning)
    }

    PC3.Button {
        id: moveRightButton
        anchors {
            bottom: parent.bottom
            right: parent.right
        }
        width: tasks.vertical ? tasks.width : tasks.height / 2
        height: tasks.vertical ? tasks.width / 2 : tasks.height
        visible: !(tasks.vertical ? taskList.atYEnd : taskList.atXEnd) || pressed
        autoRepeat: true
        icon.name: tasks.vertical ? "arrow-down" : "arrow-right"

        onClicked: taskList.positionViewAtIndex(Math.min(taskList.count - 1, taskList.itemAt(tasks.vertical ? tasks.width / 2 : taskList.contentX + moveRightButton.x, tasks.vertical ? taskList.contentY + moveRightButton.y : tasks.height / 2).itemIndex + 1), ListView.End)
    }
}
