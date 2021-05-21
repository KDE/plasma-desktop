/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.12

ShapeDoodad {
    lineWidth: 0 // indicators can be tiny. don't draw the stroke, it may overlap the fill
    fillStyle: doodad.on ? activePalette.highlight : disabledPalette.light
}
