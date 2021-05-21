/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.12
import QtQuick.Controls 2.5

ShapeCanvas {
    property QtObject doodad

    shape: doodad.shape

    x: doodad.left
    y: doodad.top
    z: doodad.priority
    // Unlike sections we'll rotate doodads in whatever way desired.
    // They do not contain anything that could be up-side-down
    // "accidentally".
    rotation: doodad.angle

    strokeSyle: disabledPalette.shadow
    fillStyle: doodad.outlineOnly ? null : disabledPalette.button
}
