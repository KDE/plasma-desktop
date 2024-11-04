/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCMUtils

import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        Kirigami.FormData.label: i18nc("@option:check", "Sticky keys:")
        Kirigami.FormData.buddyFor: stickyKeysEnable
        QQC2.CheckBox {
            id: stickyKeysEnable
            Layout.fillWidth: true
            text: i18nc("@option:check Enable sticky keys", "Enable")

            KCMUtils.SettingStateBinding {
                configObject: kcm.keyboardSettings
                settingName: "StickyKeys"
            }

            checked: kcm.keyboardSettings.stickyKeys
            onToggled: kcm.keyboardSettings.stickyKeys = checked
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "Modifier keys like Ctrl, Shift, Alt, and the Meta/Super/Windows key act as though they “stick in place” and no longer need to be held down when typing a keyboard shortcut.")
        }
    }
    RowLayout {
        QQC2.CheckBox {
            Layout.fillWidth: true
            text: i18nc("@option:check", "Lock sticky keys")

            KCMUtils.SettingStateBinding {
                configObject: kcm.keyboardSettings
                settingName: "StickyKeysLatch"
                extraEnabledConditions: kcm.keyboardSettings.stickyKeys
            }

            checked: kcm.keyboardSettings.stickyKeysLatch
            onToggled: kcm.keyboardSettings.stickyKeysLatch = checked
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "Pressing a modifier key twice will keep it in active state until pressed again.")
        }
    }
    QQC2.CheckBox {
        id: stickyKeysAutoOff
        Layout.fillWidth: true
        text: i18n("Disable when two keys are held down")

        KCMUtils.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "StickyKeysAutoOff"
            extraEnabledConditions: kcm.keyboardSettings.stickyKeys
        }

        checked: kcm.keyboardSettings.stickyKeysAutoOff
        onToggled: kcm.keyboardSettings.stickyKeysAutoOff = checked
    }
    QQC2.CheckBox {
        Layout.fillWidth: true
        text: i18n("Ring system bell when modifier keys are used")

        KCMUtils.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "StickyKeysBeep"
            extraEnabledConditions: kcm.keyboardSettings.stickyKeys
        }

        checked: kcm.keyboardSettings.stickyKeysBeep
        onToggled: kcm.keyboardSettings.stickyKeysBeep = checked
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    RowLayout{
        spacing: Kirigami.Units.smallSpacing
        Kirigami.FormData.label: i18nc("@option:check Feedback options:", "Feedback:")
        Kirigami.FormData.buddyFor: toggleKeysBeep
        QQC2.CheckBox {
            id: toggleKeysBeep
            Layout.fillWidth: true
            text: i18nc("@option:check", "Ring system bell when locking keys are used")

            KCMUtils.SettingStateBinding {
                configObject: kcm.keyboardSettings
                settingName: "ToggleKeysBeep"
            }

            checked: kcm.keyboardSettings.toggleKeysBeep
            onToggled: kcm.keyboardSettings.toggleKeysBeep = checked
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "The locking keys are Caps Lock, Num Lock, and Scroll Lock.")
        }
    }
    QQC2.CheckBox {
        text: i18nc("@option:check", "Show notification when modifier or locking keys are used")
        Layout.fillWidth: true

        KCMUtils.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "KeyboardNotifyModifiers"
        }

        checked: kcm.keyboardSettings.keyboardNotifyModifiers
        onToggled: kcm.keyboardSettings.keyboardNotifyModifiers = checked
    }
    QQC2.Button {
        text: i18nc("@action:button", "Configure Notifications…")
        icon.name: "preferences-desktop-notification"

        onClicked: kcm.configureKNotify()
    }
}
