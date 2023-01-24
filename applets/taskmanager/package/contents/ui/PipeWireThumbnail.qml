/*
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.pipewire 0.1 as PipeWire
import org.kde.taskmanager 0.1 as TaskManager

// opacity doesn't work in the root item
Item {
    anchors.fill: parent

    readonly property alias hasThumbnail: pipeWireSourceItem.enabled

    TaskManager.ScreencastingRequest {
        id: waylandItem
        uuid: thumbnailSourceItem.winId
    }

    PipeWire.PipeWireSourceItem {
        id: pipeWireSourceItem

        enabled: false // Must be set in pipewiresourceitem.cpp so opacity animation can work
        visible: waylandItem.nodeId > 0
        nodeId: waylandItem.nodeId

        anchors.fill: parent

        opacity: enabled ? 1 : 0

        Behavior on opacity {
            OpacityAnimator {
                duration: PlasmaCore.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
    }
}
