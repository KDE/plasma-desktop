/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.12 as QQC2
import org.kde.kcm 1.3 as KCM

import org.kde.kirigami 2.3 as Kirigami

Kirigami.FormLayout {
    QQC2.CheckBox {
        Kirigami.FormData.label: i18n("Sticky keys:")
        text: i18nc("Enable sticky keys", "Enable")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "StickyKeys"
        }

        checked: kcm.keyboardSettings.stickyKeys
        onToggled: kcm.keyboardSettings.stickyKeys = checked
    }
    QQC2.CheckBox {
        text: i18nc("Lock sticky keys", "Lock")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "StickyKeysLatch"
            extraEnabledConditions: kcm.keyboardSettings.stickyKeys
        }

        checked: kcm.keyboardSettings.stickyKeysLatch
        onToggled: kcm.keyboardSettings.stickyKeysLatch = checked
    }
    QQC2.CheckBox {
        id: stickyKeysAutoOff

        text: i18n("Disable when two keys are held down")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "StickyKeysAutoOff"
            extraEnabledConditions: kcm.keyboardSettings.stickyKeys
        }

        checked: kcm.keyboardSettings.stickyKeysAutoOff
        onToggled: kcm.keyboardSettings.stickyKeysAutoOff = checked
    }
    QQC2.CheckBox {
        text: i18n("Ring system bell when modifier keys are used")

        KCM.SettingStateBinding {
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

    QQC2.CheckBox {
        Kirigami.FormData.label: i18n("Feedback:")
        text: i18n("Ring system bell when locking keys are toggled")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "ToggleKeysBeep"
        }

        checked: kcm.keyboardSettings.toggleKeysBeep
        onToggled: kcm.keyboardSettings.toggleKeysBeep = checked
    }
    QQC2.CheckBox {
        text: i18n("Show notification when modifier or locking keys are used")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "KeyboardNotifyModifiers"
        }

        checked: kcm.keyboardSettings.keyboardNotifyModifiers
        onToggled: kcm.keyboardSettings.keyboardNotifyModifiers = checked
    }
    QQC2.Button {
        id: this
        text: i18n("Configure Notifications…")
        icon.name: "preferences-desktop-notification"

        onClicked: kcm.configureKNotify(this)
    }
}
