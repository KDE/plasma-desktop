/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T

import org.kde.ksvg as KSvg
import org.kde.plasma.plasmoid

import "code/tools.js" as TaskTools

T.ProgressBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    hoverEnabled: false
    padding: 0

    from: 0
    to: 100
    value: task.smartLauncherItem.progress

    contentItem: Item {
        clip: true

        KSvg.FrameSvgItem {
            id: progressFrame

            anchors.left: parent.left
            width: parent.width * control.position
            height: parent.height

            imagePath: "widgets/tasks"
            prefix: TaskTools.taskPrefix("progress", Plasmoid.location).concat(TaskTools.taskPrefix("hover", Plasmoid.location))
        }
    }

    background: null
}
