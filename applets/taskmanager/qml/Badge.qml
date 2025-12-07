/*
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.components as PlasmaComponents3
import org.kde.kirigami as Kirigami

// This top-level item is an opaque background that goes behind the colored
// background, for contrast. It's not an Item since that it would be square,
// and not round, as required here
Rectangle {
    id: badgeRect

    property alias text: label.text
    property alias textColor: label.color
    property int number: 0

    implicitWidth: Math.max(height, Math.round(label.contentWidth + radius / 2)) // Add some padding around.
    implicitHeight: implicitWidth

    radius: height / 2

    color: Kirigami.Theme.backgroundColor

    // Colored background
    Rectangle {
        anchors.fill: parent
        radius: height / 2

        color: Qt.alpha(Kirigami.Theme.highlightColor, 0.3)
        border.color: Kirigami.Theme.highlightColor
        border.width: 1
    }

    // Number
    PlasmaComponents3.Label {
        id: label
        anchors.centerIn: parent
        width: height
        height: Math.min(Kirigami.Units.gridUnit * 2, Math.round(parent.height))
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        fontSizeMode: Text.VerticalFit
        font.pointSize: 1024
        minimumPointSize: 5
        text: {
            if (badgeRect.number < 0) {
                return i18nc("Invalid number of new messages, overlay, keep short", "—");
            } else if (badgeRect.number > 9999) {
                return i18nc("Over 9999 new messages, overlay, keep short", "9,999+");
            } else {
                return badgeRect.number.toLocaleString(Qt.locale(), 'f', 0);
            }
        }
        textFormat: Text.PlainText
    }
}
