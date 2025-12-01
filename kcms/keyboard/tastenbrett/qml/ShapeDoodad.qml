/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Controls

ShapeCanvas {
    property QtObject doodad

    shape: doodad.shape

    x: doodad.left + shape.bounds.x
    y: doodad.top + shape.bounds.y
    z: doodad.priority
    // Unlike sections we'll rotate doodads in whatever way desired.
    // They do not contain anything that could be up-side-down
    // "accidentally".
    rotation: doodad.angle

    strokeStyle: doodad.outlineOnly ? doodad.color : "transparent"
    fillStyle: doodad.outlineOnly ? null : doodad.color
}
