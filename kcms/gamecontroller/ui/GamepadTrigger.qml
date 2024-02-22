/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import Qt5Compat.GraphicalEffects

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg

import org.kde.plasma.gamecontroller.kcm

/* This is for showing a gamepad trigger button*/
Item {
    id: root

    // Which trigger this is
    required property var idx
    required property var device

    // True if this is the left, false if this is the right
    required property var isLeft 

    required property var svgItem
    required property var elementId

//    visible: root.device.hasTrigger(idx)
    readonly property var value: (isLeft ? root.device.leftTrigger : root.device.rightTrigger)

//    QQC2.ToolTip.visible: hoverHandler.hovered
//    QQC2.ToolTip.text: trigger.name

    HoverHandler {
        id: hoverHandler

        acceptedDevices: PointerDevice.Mouse
    }

    KSvg.SvgItem {
        id: icon

        visible: false
        width: Math.round(elementRect.width)
        height: Math.round(elementRect.height)
        x: Math.round(elementRect.x)
        y: Math.round(elementRect.y)

        svg: root.svgItem
        elementId: root.elementId
    }

    ColorOverlay {
        opacity: 1 - value
        anchors.fill: icon
        source: icon
        color: Kirigami.Theme.textColor
    }

    ColorOverlay {
        opacity: value
        anchors.fill: icon
        source: icon
        color: Kirigami.Theme.highlightColor
    }
}
