/*
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.4

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Rectangle {
    id: badgeRect

    property alias text: label.text
    property alias textColor: label.color
    property int number: 0

    implicitWidth: Math.max(height, Math.round(label.contentWidth + radius / 2)) // Add some padding around.
    color: PlasmaCore.Theme.highlightColor
    radius: height / 2

    PlasmaComponents.Label {
        id: label
        anchors.centerIn: parent
        width: height
        height: Math.min(PlasmaCore.Units.gridUnit * 2, Math.round(parent.height))
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        fontSizeMode: Text.VerticalFit
        font.pointSize: 1024
        minimumPointSize: 5
        color: PlasmaCore.Theme.highlightedTextColor
        text: {
            if (badgeRect.number < 0) {
                return i18nc("Invalid number of new messages, overlay, keep short", "—");
            } else if (badgeRect.number > 9999) {
                return i18nc("Over 9999 new messages, overlay, keep short", "9,999+");
            } else {
                return badgeRect.number.toLocaleString(Qt.locale(), 'f', 0);
            }
        }
    }
}
