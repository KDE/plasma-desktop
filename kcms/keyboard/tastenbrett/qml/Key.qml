/*
    Copyright 2019 Harald Sitter <sitter@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
