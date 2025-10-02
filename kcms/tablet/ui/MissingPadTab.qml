/*
    SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami

Item {
    id: root

    Kirigami.PlaceholderMessage {
        text: i18nd("kcm_tablet", "No pad inputs found")
        explanation: i18n("There's no buttons or other inputs detected. This could mean you're missing a driver for this device.")
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }
}
