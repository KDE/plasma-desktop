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
import "."

Rectangle {
    id: root
    color: Kirigami.Theme.backgroundColor

    required property var device
    required property string svgPath

    function resize() {
        image.resize(root.width, root.height)
    }
    
    KSvg.Svg {
        id: image

        imagePath: svgPath
        size.width: root.width
        size.height: root.height
    }

    ColorOverlay {
        anchors.fill: base

        source: base
        color: Kirigami.Theme.disabledTextColor
    }

    KSvg.SvgItem {
        id: base

        visible: false
        anchors.fill: parent

        svg: image
        elementId: "base"
    }

    GamepadTrigger {
        idx: GamepadButton.SDL_CONTROLLER_AXIS_TRIGGERRIGHT
        isLeft: false
        device: root.device
        svgItem: image
        elementId: "right-trigger"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
        device: root.device
        svgItem: image
        elementId: "right-shoulder"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_LEFTSHOULDER
        device: root.device
        svgItem: image
        elementId: "left-shoulder"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_START
        device: root.device
        svgItem: image
        elementId: "mid-right"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_BACK
        device: root.device
        svgItem: image
        elementId: "mid-left"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_DPAD_UP
        device: root.device
        svgItem: image
        elementId: "up"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_DPAD_RIGHT
        device: root.device
        svgItem: image
        elementId: "right"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_DPAD_DOWN
        device: root.device
        svgItem: image
        elementId: "down"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_DPAD_LEFT
        device: root.device
        svgItem: image
        elementId: "left"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_GUIDE
        device: root.device
        svgItem: image
        elementId: "center"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_X
        device: root.device
        svgItem: image
        elementId: "x-button"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_Y
        device: root.device
        svgItem: image
        elementId: "y-button"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_A
        device: root.device
        svgItem: image
        elementId: "a-button"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_B
        device: root.device
        svgItem: image
        elementId: "b-button"
    }

    GamepadTrigger {
        idx: GamepadButton.SDL_CONTROLLER_AXIS_TRIGGERLEFT
        isLeft: true
        device: root.device
        svgItem: image
        elementId: "left-trigger"
    }

    GamepadStick {
        idx: GamepadButton.SDL_CONTROLLER_AXIS_LEFTX
        buttonidx: GamepadButton.SDL_CONTROLLER_BUTTON_LEFTSTICK
        isLeftAxis: true
        device: root.device
        svgItem: image
        elementId: "l-pad"
    }

    GamepadStick {
        idx: GamepadButton.SDL_CONTROLLER_AXIS_RIGHTX
        buttonidx: GamepadButton.SDL_CONTROLLER_BUTTON_RIGHTSTICK
        isLeftAxis: false
        device: root.device
        svgItem: image
        elementId: "r-pad"
    }
}
