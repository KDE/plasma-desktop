/*
    SPDX-FileCopyrightText: 2025 Oliver Beard <olib141@outlook.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

ColumnLayout {
    spacing: Kirigami.Units.smallSpacing

    Kirigami.FormLayout {
        id: formLayout

        QQC2.CheckBox {
            id: invertBox
            Kirigami.FormData.label: i18nc("@label", "Invert:")
            text: i18nc("@option check, Enable invert effect", "Enable")

            KCM.SettingStateBinding {
                configObject: kcm.invertSettings
                settingName: "Invert"
            }

            checked: kcm.invertSettings.invert
            onToggled: kcm.invertSettings.invert = checked
        }

        QQC2.Label {
            leftPadding: Application.layoutDirection === Qt.LeftToRight ? invertBox.contentItem.leftPadding : padding
            rightPadding: Application.layoutDirection === Qt.RightToLeft ? invertBox.contentItem.rightPadding : padding
            enabled: invertBox.checked
            text: i18nc("@label Hint for invert effect enable button", "Use shortcuts to toggle inverting display and window colors")
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            font: Kirigami.Theme.smallFont
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.Button {
            text: i18nc("@action:button", "Configure Shortcuts…")
            icon.name: "preferences-desktop-keyboard-shortcut"

            enabled: invertBox.checked

            onClicked: kcm.configureInvertShortcuts()
        }
    }
}
