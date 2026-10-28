/*
    SPDX-FileCopyrightText: 2017 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.VirtualKeyboard as QtVK

/*
 * Qt Virtual Keyboard's input panel. Instantiating it is what keeps Qt from
 * creating its own DesktopInputPanel, which pops up whenever a text field
 * gains focus, mouse clicks included.
 *
 * The panel has no animation of its own: VirtualKeyboard.qml slides and fades
 * the loader instead. That way the keyboard also animates out when Qt hides
 * the input method on its own, which its built-in hide key does.
 */
QtVK.InputPanel {
    id: inputPanel
    property bool activated: false
    active: activated && InputMethod.visible
    // Stay visible while the loader slides the panel off-screen. Hiding it as
    // soon as InputMethod.visible turns false, which Qt Virtual Keyboard's own
    // hide key does, makes the keyboard disappear in a single frame.
    visible: activated
    width: parent.width
}
