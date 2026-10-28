/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T

import org.kde.kirigami as Kirigami
import org.kde.plasma.workspace.keyboardlayout as Keyboards

/*
 * Shows and hides the virtual keyboard and keeps the login form above it.
 *
 * On Wayland the keyboard is drawn by KWin, so all we do is forward to
 * Keyboards.KWinVirtualKeyboard. On X11, which is still what SDDM uses by
 * default, there is no KWin to talk to, so the greeter falls back to Qt
 * Virtual Keyboard instead.
 *
 * Instantiating Qt Virtual Keyboard's input panel is also what keeps Qt from
 * creating its own DesktopInputPanel, which pops up whenever a text field
 * gains focus, mouse clicks included. See
 * https://bugs.kde.org/show_bug.cgi?id=527138
 */
Loader {
    id: loader

    // Usually, anchors on a top-level component is a bad idea, but this is a
    // tightly integrated component sharing the screen with the theme.
    anchors {
        left: parent.left
        right: parent.right
    }

    required property Item screenRoot
    required property T.StackView mainStack
    required property /* MainBlock | Login */ Item mainBlock
    required property Item passwordField

    // Whether the keyboard is currently on screen.
    readonly property bool keyboardVisible: active ? state === "visible" : Keyboards.KWinVirtualKeyboard.visible
    // Whether this platform has a virtual keyboard at all.
    readonly property bool available: active ? status === Loader.Ready : Keyboards.KWinVirtualKeyboard.available
    readonly property bool keyboardActive: (item as QtVirtualKeyboard)?.active ?? false

    active: !Qt.platform.pluginName.includes("wayland")
    source: Qt.resolvedUrl("QtVirtualKeyboard.qml")

    function toggle() {
        if (!active) {
            if (Keyboards.KWinVirtualKeyboard.visible) {
                Keyboards.KWinVirtualKeyboard.active = false;
            } else {
                // Otherwise the password field loses focus and on-screen
                // keyboard keystrokes get eaten
                passwordField.forceActiveFocus();
                Keyboards.KWinVirtualKeyboard.forceActivate();
            }
            return;
        }
        if (!item) {
            return;
        }
        if (state === "hidden") {
            // Otherwise the password field loses focus and on-screen keyboard
            // keystrokes get eaten
            passwordField.forceActiveFocus();
            state = "visible";
        } else {
            state = "hidden";
        }
    }

    onKeyboardActiveChanged: {
        // The keyboard was hidden by something other than the button, e.g. Qt
        // Virtual Keyboard's own hide key, which turns InputMethod.visible off
        // before the theme is notified. Animate it out.
        if (!keyboardActive && state === "visible") {
            state = "hidden";
        }
    }

    state: "hidden"

    states: [
        State {
            name: "visible"
            PropertyChanges {
                mainStack.y: Math.min(0, screenRoot.height - loader.height - mainBlock.visibleBoundary)
            }
            PropertyChanges {
                loader.y: screenRoot.height - loader.height
                loader.opacity: 1
            }
        },
        State {
            name: "hidden"
            PropertyChanges {
                mainStack.y: 0
            }
            PropertyChanges {
                loader.y: screenRoot.height
                loader.opacity: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "hidden"
            to: "visible"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        loader.item.activated = true;
                        Qt.inputMethod.show();
                    }
                }
                // The loader slides the keyboard in from below the screen
                // while fading it in, and moves the form out of the way
                ParallelAnimation {
                    NumberAnimation {
                        target: loader.mainStack
                        property: "y"
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: loader
                        property: "y"
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.OutQuad
                    }
                    NumberAnimation {
                        target: loader
                        property: "opacity"
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.OutQuad
                    }
                }
            }
        },
        Transition {
            from: "visible"
            to: "hidden"
            SequentialAnimation {
                ParallelAnimation {
                    NumberAnimation {
                        target: loader.mainStack
                        property: "y"
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                    // Nothing animates the keyboard out of the way, so slide
                    // it off-screen and fade it out ourselves
                    NumberAnimation {
                        target: loader
                        property: "y"
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InQuad
                    }
                    NumberAnimation {
                        target: loader
                        property: "opacity"
                        duration: Kirigami.Units.longDuration
                        easing.type: Easing.InQuad
                    }
                }
                // Deactivate only once the keyboard has slid out, otherwise it
                // vanishes before the animation starts
                ScriptAction {
                    script: {
                        loader.item.activated = false;
                        Qt.inputMethod.hide();
                    }
                }
            }
        }
    ]
}
