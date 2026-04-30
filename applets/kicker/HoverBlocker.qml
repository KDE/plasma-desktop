/*
 *  SPDX-FileCopyrightText: 2026 Christoph Wolk <cwo.kde@posteo.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
pragma ComponentBehavior: Bound

import QtQuick

MouseArea {
    id: hoverBlock  // don't hover-activate until mouse is moved to not interfere with keyboard use

    property bool mouseMoved: false

    hoverEnabled: true
    propagateComposedEvents: true // clicking should still work if hovering is blocked

    onPositionChanged: if (!mouseMoved) {
        mouseMoved = true
    } else {
        enabled = false // this immediately triggers other hover events when bound to their hoverEnabled
    }

    onPressed: event => {
        enabled = false
        event.accepted = false
    }

    function reset() {
        mouseMoved = false
        enabled = true
    }
}
