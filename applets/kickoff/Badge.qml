/*
    SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.plasma.components as PC3
import org.kde.kirigami as Kirigami

PC3.Frame {
    id: badge

    property alias text: badgeLabel.text

    leftPadding: text !== "" ? Kirigami.Units.cornerRadius : height / 2
    rightPadding: text !== "" ? Kirigami.Units.cornerRadius : height / 2

    background: Rectangle {
        implicitHeight: badge.text !== "" ? Math.round(Kirigami.Units.iconSizes.sizeForLabels * 1.3) : 0
        radius: badge.text !== "" ? Kirigami.Units.cornerRadius : height / 2
        color: badge.text !== "" ? Kirigami.Theme.positiveBackgroundColor : Kirigami.Theme.positiveTextColor
    }

    Item {
        implicitWidth: badgeLabel.text !== "" ? badgeLabel.implicitWidth : 0
        implicitHeight: badgeLabel.text !== "" ? badgeLabel.implicitHeight : 0

        PC3.Label {
            id: badgeLabel
            text: badge.text
            color: Kirigami.Theme.textColor
            font.bold: true
            font.pixelSize: Kirigami.Theme.smallFont.pixelSize
            visible: text !== ""
        }
    }
}
