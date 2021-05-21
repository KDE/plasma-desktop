/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.12
import QtQuick.Controls 2.5

Label {
    property QtObject doodad

    x: doodad.left
    y: doodad.top
    z: doodad.priority
    // random note: the width is much larger than desired on pc geoms because
    //   the geom only defines left anchors and has a forced \n inside
    width: doodad.width
    height: doodad.height
    rotation: doodad.angle
    text: doodad.text
    color: disabledPalette.buttonText

    fontSizeMode: Text.Fit
    minimumPixelSize: 6
    font.pixelSize: height
}
