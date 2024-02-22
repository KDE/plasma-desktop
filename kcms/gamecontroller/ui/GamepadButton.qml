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

/* This is for a single gamepad button
   In order to make it relatively simpler to show
   a gamepad for showing button state and arrangement.
*/
Item {
    id: root

    // Which button this is
    required property var idx
    required property var device

    readonly property var buttonState: root.device.button(idx).state

    required property var svgItem
    required property var elementId

//    visible: root.device.hasButton(idx)

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
        anchors.fill: icon

        source: icon
        color: (root.buttonState) ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
    }
}
