/*
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.plasma.private.kcm_mouse as Mouse

Kirigami.InlineMessage {
    required property Mouse.message message

    position: Kirigami.InlineMessage.Position.Header
    visible: message.type !== Mouse.MessageType.None
    text: message.text
    type: {
        switch (message.type) {
        case Mouse.MessageType.Error:
            return Kirigami.MessageType.Error;
        case Mouse.MessageType.Information:
        case Mouse.MessageType.None:
        default:
            return Kirigami.MessageType.Information;
        }
    }
}
