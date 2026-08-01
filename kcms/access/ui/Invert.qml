/*
    SPDX-FileCopyrightText: 2025 Oliver Beard <olib141@outlook.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

Kirigami.Form {
    Kirigami.FormGroup {
        Kirigami.FormEntry {
            title: i18nc("@label", "Invert:")
            subtitle: i18nc("@label Hint for invert effect enable button", "Use shortcuts to toggle inverting display and window colors")
            contentItem: QQC2.CheckBox {
                id: invertBox
                text: i18nc("@option check, Enable invert effect", "Enable")

                KCM.SettingStateBinding {
                    configObject: kcm.invertSettings
                    settingName: "Invert"
                }

                checked: kcm.invertSettings.invert
                onToggled: kcm.invertSettings.invert = checked
            }
        }

        Kirigami.FormSeparator {}

        Kirigami.FormAction {
            enabled: action.enabled
            action: QQC2.Action {
                enabled: invertBox.checked
                text: i18nc("@action:button", "Configure Shortcuts…")
                icon.name: "preferences-desktop-keyboard-shortcut"
                onTriggered: kcm.configureInvertShortcuts()
            }
        }
    }
}
