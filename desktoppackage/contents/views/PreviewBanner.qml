/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.workspace.components 2.0 as WorkspaceComponents
import org.kde.kirigami 2.20 as Kirigami

Item {
    width: childrenRect.width
    height: childrenRect.height

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    TapHandler {
        onTapped: Qt.openUrlExternally("https://bugs.kde.org/")
    }

    WorkspaceComponents.ShadowedLabel {
        id: title
        anchors {
            top: parent.top
            right: parent.right
        }
        z: 2
        text: desktop.previewBannerTitle
        // Emulate the size of a level 1 heading
        font.pointSize: Math.round(Kirigami.Theme.defaultFont.pointSize * 1.35)
    }

    WorkspaceComponents.ShadowedLabel {
        anchors {
            top: title.bottom
            right: parent.right
        }
        z: 2
        text: desktop.previewBannerText
    }
}
