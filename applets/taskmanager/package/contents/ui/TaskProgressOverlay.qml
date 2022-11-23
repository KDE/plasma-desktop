/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Templates 2.15 as T

import org.kde.plasma.core 2.0 as PlasmaCore

import "code/tools.js" as TaskTools

T.ProgressBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    hoverEnabled: false
    padding: 0

    contentItem: Item {
        clip: true

        PlasmaCore.FrameSvgItem {
            id: progressFrame

            anchors.left: parent.left
            width: parent.width * control.position
            height: parent.height

            imagePath: "widgets/tasks"
            prefix: TaskTools.taskPrefix("progress", plasmoid.location).concat(TaskTools.taskPrefix("hover", plasmoid.location))
        }
    }

    background: null
}
