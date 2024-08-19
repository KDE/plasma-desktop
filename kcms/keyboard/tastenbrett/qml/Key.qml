/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2024 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Controls

ShapeCanvas {
    id: root

    property QtObject key
    property color keyColor: key.pressed ? activePalette.highlight : key.color || activePalette.button
    property color labelColor: key.pressed ? activePalette.highlightedText : key.textColor || activePalette.buttonText

    shape: key ? key.shape : null
    lineWidth: 1
    strokeStyle: "transparent"
    fillStyle: keyColor

    onKeyColorChanged: requestPaint()

    KeyCap {
        key: parent.key
        complexShape: shape?.outlines[0].points.length > 4 ?? false

        anchors.fill: parent
        anchors.margins: 22 // arbitrary spacing to key outlines
    }

    Component.onCompleted: {
        if (!parent || !parent.row) {
            // There's implicit layout logic below when used inside a row.
            // Key may also be used standalone, so skip the layout bits.
            return;
        }

        const siblings = parent.children.filter(item => item !== this);
        if (parent.row.orientation === Qt.Horizontal) {
            const previousX = Math.max(...siblings.map(item => item.x + item.width),  0);
            x = previousX + key.gap;
            y = shape.bounds.y;
        } else {
            const previousY = Math.max(...siblings.map(item => item.y + item.height), 0);
            y = previousY + key.gap;
            x = shape.bounds.y;
        }
    }
}
