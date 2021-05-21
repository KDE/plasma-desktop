/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.12
import QtQuick.Controls 2.5

ShapeCanvas {
    id: root

    property QtObject key
    property variant keyColor: key.pressed ? activePalette.highlight : activePalette.button
    property variant labelColor: key.pressed ? activePalette.highlightedText : activePalette.buttonText

    shape: key ? key.shape : null
    strokeSyle: activePalette.shadow
    fillStyle: keyColor

    onKeyColorChanged: requestPaint()

    KeyCap {
        key: parent.key

        anchors.fill: parent
        anchors.margins: 22 // arbitrary spacing to key outlines
    }

    Component.onCompleted: {
        if (!parent || !parent.row) {
            // There's implicit layout logic below when used inside a row.
            // Key may also be used standalone, so skip the layout bits.
            return;
        }

        if (parent.row.orientation === Qt.Horizontal) {
            x = 0

            for (var i in parent.children) {
                // find the furthest sibling -> it is our nearst one
                var sibling = parent.children[i]
                if (sibling === this) {
                    continue
                }
                x = Math.max(x, sibling.x + sibling.width)
            }
            if (x > 0) {
                x += key.gap // found a sibling, gap us from it
            }

            y = shape.bounds.y
        } else {
            y = 0

            for (var i in parent.children) {
                // find the furthest sibling -> it is our nearst one
                var sibling = parent.children[i]
                if (sibling === this) {
                    continue
                }
                y = Math.max(y, sibling.y + sibling.height)
            }
            if (y > 0) {
                y += key.gap // found a sibling, gap us from it
            }

            x = shape.bounds.x
        }
    }
}
