/*
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
    SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami

import org.kde.plasma.private.kcm_clock as DateTime

ColumnLayout {
    id: root

    property list<DateTime.message> messages

    spacing: 0

    Repeater {
        model: root.messages
        delegate: Kirigami.InlineMessage {
            required property DateTime.message modelData

            Layout.fillWidth: true

            position: Kirigami.InlineMessage.Position.Header
            visible: modelData.type !== DateTime.message.None
            text: modelData.text
            type: {
                switch (modelData.type) {
                    case DateTime.message.Error:
                        return Kirigami.MessageType.Error;
                    case DateTime.message.Information:
                    case DateTime.message.None:
                    default:
                        return Kirigami.MessageType.Information;
                }
            }
        }
    }
}
