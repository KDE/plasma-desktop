/*
    SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami

Item {
    id: root

    Kirigami.PlaceholderMessage {
        text: i18ndc("kcm_tablet", "@info placeholdermessage main text", "No pad inputs found")
        explanation: i18nc("@info:usagetip placeholdermessage explanation", "No buttons or other inputs were detected. This could mean the driver for this device is missing.")
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }
}
