/*
    SPDX-FileCopyrightText: 2026 Oliver Beard <olib141@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

PlasmaComponents.ComboBox {
    id: root

    model: sessionModel
    textRole: "name"

    visible: count > 1
    flat: true
    displayText: i18nd("plasma_login", "Desktop Session: %1", currentText)

    contentItem: QQC2.Label {
        font: root.font
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight

        text: root.displayText
    }

    PlasmaComponents.ToolTip.text: currentText
    PlasmaComponents.ToolTip.visible: hovered && contentItem.truncated && !popup.visible
    PlasmaComponents.ToolTip.delay: Kirigami.Units.toolTipDelay
}
