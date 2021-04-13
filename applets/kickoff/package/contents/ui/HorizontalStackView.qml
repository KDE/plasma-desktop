/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import org.kde.plasma.core 2.0 as PlasmaCore

T.StackView {
    id: root
    property bool reverseTransitions: false
    property bool movementTransitionsEnabled: true
    implicitWidth: implicitContentWidth + leftPadding + rightPadding
    implicitHeight: implicitContentHeight + topPadding + bottomPadding
    clip: busy
    contentItem: currentItem
    Accessible.ignored: true
    popEnter: enterTransition
    popExit: exitTransition
    pushEnter: enterTransition
    pushExit: exitTransition
    replaceEnter: enterTransition
    replaceExit: exitTransition
    // Using NumberAnimation instead of XAnimator because the latter wasn't always smooth enough
    Transition {
        id: enterTransition
        NumberAnimation {
            properties: "x"
            from: (root.reverseTransitions ? -0.5 : 0.5) * (root.mirrored ? -1 : 1) * -root.width
            to: 0
            duration: root.movementTransitionsEnabled ? PlasmaCore.Units.longDuration : 0
            easing.type: Easing.OutCubic
        }
        NumberAnimation { property: "opacity"
            from: 0.0
            to: 1.0
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }
    Transition {
        id: exitTransition
        NumberAnimation {
            property: "x"
            from: 0
            to: (root.reverseTransitions ? -0.5 : 0.5) * (root.mirrored ? -1 : 1) * root.width
            duration: root.movementTransitionsEnabled ? PlasmaCore.Units.longDuration : 0
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            property: "opacity"
            from: 1.0
            to: 0.0
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }
}
