/**
 * SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import org.kde.kirigami as Kirigami

// Copy of Kirigami.Dialog modal overlay, with adjustments for KCM footer margin.
Item {
    id: background

    // to account for extra margin in KCM footer
    property int bottomMargin: 8

    Kirigami.Separator {
        id: separator
        visible: false
    }

    // black background, fades in and out
    Rectangle {
        color: Qt.rgba(0, 0, 0, 0.3)
        anchors {
            fill: parent
            // Don't cover the extra footer margin area to avoid eye-bleeding
            // experience. KCM doesn't have control over user's interactions
            // in that area anyway.
            bottomMargin: background.bottomMargin + separator.implicitHeight
        }
    }

    // the opacity of the item is changed internally by QQuickPopup on open/close
    Behavior on opacity {
        OpacityAnimator {
            duration: Kirigami.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
}
