/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.12
import QtQuick.Controls 2.5

Label {
    elide: Text.ElideRight
    color: labelColor // from parent scope

    fontSizeMode: Text.Fit
    minimumPixelSize: 6
    font.pixelSize: parent.height
}
