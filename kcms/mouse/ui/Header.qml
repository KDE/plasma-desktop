/*
    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick.Layouts
import org.kde.plasma.private.kcm_mouse as Mouse

ColumnLayout {
    id: root

    required property Mouse.message saveLoadMessage
    required property Mouse.message hotplugMessage

    spacing: 0

    Message {
        Layout.fillWidth: true
        message: root.saveLoadMessage
    }

    Message {
        Layout.fillWidth: true
        message: root.hotplugMessage
        showCloseButton: true
    }
}
