/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

T.StackView {
    id: root
    property bool reverseTransitions: false
    property bool movementTransitionsEnabled: false
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
    // Using NumberAnimation instead of YAnimator because the latter wasn't always smooth enough
    Transition {
        id: enterTransition
        NumberAnimation {
            properties: "y"
            from: (root.reverseTransitions ? -0.5 : 0.5) * -root.height
            to: 0
            duration: root.movementTransitionsEnabled ? Kirigami.Units.longDuration : 0
            easing.type: Easing.OutCubic
        }
        NumberAnimation { property: "opacity"
            from: 0.0
            to: 1.0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }
    Transition {
        id: exitTransition
        NumberAnimation {
            property: "y"
            from: 0
            to: (root.reverseTransitions ? -0.5 : 0.5) * root.height
            duration: root.movementTransitionsEnabled ? Kirigami.Units.longDuration : 0
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            property: "opacity"
            from: 1.0
            to: 0.0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }

    // NOTE: Not using animations because they tended to look too jumpy
}
