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
        Kirigami.FormData.label: i18n("Slow keys:")
        Kirigami.FormData.buddyFor: slowKeys
        QQC2.CheckBox {
            id: slowKeys

            text: i18nc("Enable slow keys", "Enable")

            KCMUtils.SettingStateBinding {
                configObject: kcm.keyboardFiltersSettings
                settingName: "SlowKeys"
            }

            checked: kcm.keyboardFiltersSettings.slowKeys
            onToggled: kcm.keyboardFiltersSettings.slowKeys = checked
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "For a key to be accepted, it has to be held until the set amount of time. Useful if you accidentally type more than one key at a time or have difficulty pressing the key you want the first time.")
        }
    }

    QQC2.SpinBox {
        id: slowKayDelay

        Kirigami.FormData.label: i18nc("Slow keys Delay", "Delay:")

        KCMUtils.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "SlowKeysDelay"
            extraEnabledConditions: kcm.keyboardFiltersSettings.slowKeys
        }

        from: 100
        to: 10000

        value: kcm.keyboardFiltersSettings.slowKeysDelay
        onValueModified: kcm.keyboardFiltersSettings.slowKeysDelay = value

        textFromValue: function(value, locale) {
            return i18np("%1 ms", "%1 ms", value)
        }

        valueFromText: (text, locale) => {
            return Number.fromLocaleString(locale, text.replace(i18ncp("short for millisecond(s)", "ms", "ms"), ""))
        }
    }
    Item {
        Kirigami.FormData.isSection: true
    }
    QQC2.CheckBox {
        id: slowKeysPressBeep

        Kirigami.FormData.label: i18n("Ring system bell:")
        text: i18nc("Use system bell when a key is pressed", "when any key is &pressed")

        KCMUtils.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "SlowKeysPressBeep"
            extraEnabledConditions: kcm.keyboardFiltersSettings.slowKeys
        }

        checked: kcm.keyboardFiltersSettings.slowKeysPressBeep
        onToggled: kcm.keyboardFiltersSettings.slowKeysPressBeep = checked
    }
    QQC2.CheckBox {
        id: slowKeysAcceptBeep

        text: i18nc("Use system bell when a key is accepted", "when any key is &accepted")

        KCMUtils.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "SlowKeysAcceptBeep"
            extraEnabledConditions: kcm.keyboardFiltersSettings.slowKeys
        }

        checked: kcm.keyboardFiltersSettings.slowKeysAcceptBeep
        onToggled: kcm.keyboardFiltersSettings.slowKeysAcceptBeep = checked
    }
    QQC2.CheckBox {
        id: slowKeysRejectBeep

        text: i18nc("Use system bell when a key is rejected", "when any key is &rejected")

        KCMUtils.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "SlowKeysRejectBeep"
            extraEnabledConditions: kcm.keyboardFiltersSettings.slowKeys
        }

        checked: kcm.keyboardFiltersSettings.slowKeysRejectBeep
        onToggled: kcm.keyboardFiltersSettings.slowKeysRejectBeep = checked
    }
    Item {
        Kirigami.FormData.isSection: true
    }
    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        Kirigami.FormData.label: i18n("Bounce keys:")
        Kirigami.FormData.buddyFor: bounceKeys
        QQC2.CheckBox {
            id: bounceKeys

            text: i18nc("Bounce keys enable", "Enable");

            KCMUtils.SettingStateBinding {
                configObject: kcm.keyboardFiltersSettings
                settingName: "BounceKeys"
            }

            checked: kcm.keyboardFiltersSettings.bounceKeys
            onToggled: kcm.keyboardFiltersSettings.bounceKeys = checked
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18nc("@info:tooltip", "Ignore rapid, repeated keypresses of the same key. Useful if you have hand tremors that cause you to press a key multiple times when you only intend to press once.")
        }
    }

    QQC2.SpinBox {
        id: bounceKeysDelay

        Kirigami.FormData.label: i18nc("Bounce keys delay", "Delay:")

        KCMUtils.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "BounceKeysDelay"
            extraEnabledConditions: kcm.keyboardFiltersSettings.bounceKeys
        }

        from: 100
        to: 10000

        value: kcm.keyboardFiltersSettings.bounceKeysDelay
        onValueModified: kcm.keyboardFiltersSettings.bounceKeysDelay = value

        textFromValue: function(value, locale) {
            return i18np("%1 ms", "%1 ms", value)
        }

        valueFromText: (text, locale) => {
            return Number.fromLocaleString(locale, text.replace(i18ncp("short for millisecond(s)", "ms", "ms"), ""))
        }
    }

    QQC2.CheckBox {
        id: bounceKeysRejectBeep

        text: i18n("Ring system bell when rejected")

        KCMUtils.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "BounceKeysRejectBeep"
            extraEnabledConditions: kcm.keyboardFiltersSettings.bounceKeys
        }

        checked: kcm.keyboardFiltersSettings.bounceKeysRejectBeep
        onToggled: kcm.keyboardFiltersSettings.bounceKeysRejectBeep = checked
    }
}
