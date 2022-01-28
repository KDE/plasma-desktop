/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore

import "code/tools.js" as TaskTools

Item {
    id: background

    Item {
        id: progress
        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
        }

        width: parent.width * (task.smartLauncherItem.progress / 100)
        clip: true

        PlasmaCore.FrameSvgItem {
            id: progressFrame
            width: background.width
            height: background.height

            imagePath: "widgets/tasks"
            prefix: TaskTools.taskPrefix("progress").concat(TaskTools.taskPrefix("hover"))
        }
    }
}
