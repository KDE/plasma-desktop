/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import Qt5Compat.GraphicalEffects

import org.kde.kirigami 2.20 as Kirigami

Item {
    width: childrenRect.width
    height: childrenRect.height
    opacity: 0.8

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    TapHandler {
        onTapped: Qt.openUrlExternally("https://bugs.kde.org/")
    }

    Kirigami.Heading {
        id: title
        anchors {
            top: parent.top
            right: parent.right
        }
        z: 2
        color: "#fff"
        horizontalAlignment: Text.AlignLeft
        text: desktop.previewBannerTitle
    }

    DropShadow {
        anchors.fill: title
        z: 1
        color: "black"
        horizontalOffset: 0
        verticalOffset: 1
        opacity: 0.5
        radius: 4.0
        samples: radius * 2 + 1
        spread: 0.35
        source: title
    }

    QQC2.Label {
        id: textLabel
        anchors {
            top: title.bottom
            right: parent.right
        }
        z: 2
        color: "#fff"
        horizontalAlignment: Text.AlignLeft
        text: desktop.previewBannerText
    }

    DropShadow {
        anchors.fill: textLabel
        z: 1
        color: "black"
        horizontalOffset: 0
        verticalOffset: 1
        opacity: 0.5
        radius: 4.0
        samples: radius * 2 + 1
        spread: 0.35
        source: textLabel
    }
}