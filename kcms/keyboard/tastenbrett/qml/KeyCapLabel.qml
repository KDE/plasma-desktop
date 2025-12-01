/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Controls

Label {
    color: labelColor // from parent scope

    elide: Text.ElideRight
    wrapMode: Text.WordWrap
    maximumLineCount: 2
    textFormat: Text.PlainText

    fontSizeMode: Text.Fit
    minimumPixelSize: 6
    font.pixelSize: text.length === 1 ? 80 : 40

    renderType: Text.QtRendering
}
