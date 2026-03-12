/*
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.components as PlasmaComponents3
import org.kde.kirigami as Kirigami

// This top-level item is an opaque background that goes behind the colored
// badge, for contrast. It's not an Item since that it would be square,
// and not round, as required here
Rectangle {
    id: badgeRect

    property alias text: badge.text
    property int number: 0

    implicitWidth: badge.width
    implicitHeight: badge.height

    radius: height / 2

    color: Kirigami.Theme.backgroundColor

    // The actual badge
    Kirigami.Badge {
        id: badge

        anchors.centerIn: parent

        padding: 0

        text: {
            if (badgeRect.number > 9999) {
                return i18nc("Over 9999 new messages, overlay, keep short", "9,999+");
            } else {
                return badgeRect.number.toLocaleString(Qt.locale(), 'f', 0);
            }
        }
    }
}
