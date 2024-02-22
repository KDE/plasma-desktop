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

/* This is for showing a gamepad axis */
Item {
    id: root

    // Which axis this is
    required property var idx
    required property var buttonidx
    required property var device
    required property var isLeftAxis

    readonly property var axis: (isLeftAxis ? root.device.leftAxis : root.device.rightAxis)
    readonly property var buttonState: root.device.button(buttonidx).state

    required property var svgItem
    required property var elementId

//    visible: root.device.hasAxis(idx)

    KSvg.SvgItem {
        id: icon

        visible: false
        width: Math.round(elementRect.width)
        height: Math.round(elementRect.height)
        x: Math.round(elementRect.x) + (axis.x * 10)
        y: Math.round(elementRect.y) + (axis.y * 10)

        svg: root.svgItem
        elementId: root.elementId

    }

    KSvg.SvgItem {
        id: outline

        visible: false
        width: Math.round(elementRect.width)
        height: Math.round(elementRect.height)
        x: Math.round(elementRect.x)
        y: Math.round(elementRect.y)

        svg: root.svgItem
        elementId: root.elementId + '-outline'
    }

    ColorOverlay {
        anchors.fill: icon

        source: icon
        color: root.buttonState ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
    }

    ColorOverlay {
        anchors.fill: outline

        source: outline
        color: Kirigami.Theme.textColor
    }
}
