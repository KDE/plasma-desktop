/*
    SPDX-FileCopyrightText: 2026 Ramil Nurmanov <ramil2004nur@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.Icon {
    anchors.fill: parent
    source: toolTipDelegate.icon
    animated: false
    visible: valid
}
