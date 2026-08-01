/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCMUtils

import org.kde.kirigami as Kirigami
import org.kde.plasma.access.kcm

Kirigami.Form {
    Kirigami.FormGroup {
        Kirigami.FormEntry {
            title: i18nc("@title:group prefix", "Slow keys:")
            contentItem: QQC2.CheckBox {
                id: slowKeys
                text: i18nc("@option:check Enable slow keys", "Enable")

                KCMUtils.SettingStateBinding {
                    configObject: kcm.keyboardFiltersSettings
                    settingName: "SlowKeys"
                }

                checked: kcm.keyboardFiltersSettings.slowKeys
                onToggled: kcm.keyboardFiltersSettings.slowKeys = checked
            }
            trailingItems: [
                Kirigami.ContextualHelpButton {
                    toolTipText: i18nc("@info:tooltip", "For a key to be accepted, it has to be held until the set amount of time. Useful if you accidentally type more than one key at a time or have difficulty pressing the key you want the first time.")
                }
            ]
        }

        Kirigami.FormEntry {
            title: i18nc("@label:spinbox Slow keys Delay", "Delay:")
            contentItem: QQC2.SpinBox {
                id: slowKeyDelay


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
                    return i18ncp("@label:valuesuffix %1 is slow keys delay", "%1 ms", "%1 ms", value)
                }

                validator: IntValidatorWithSuffix {
                    bottom: slowKeyDelay.from
                    top: slowKeyDelay.to
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18ncp("@label:valuesuffix short for millisecond(s)", "ms", "ms"), ""))
                }
            }
        }
        Kirigami.FormSeparator {}
        Kirigami.FormEntry {
            title: i18nc("@title:group prefix for checkbox group", "Ring system bell:")
            contentItem: QQC2.CheckBox {
                id: slowKeysPressBeep

                text: i18nc("@option:check Use system bell when a key is pressed", "when any key is &pressed")

                KCMUtils.SettingStateBinding {
                    configObject: kcm.keyboardFiltersSettings
                    settingName: "SlowKeysPressBeep"
                    extraEnabledConditions: kcm.keyboardFiltersSettings.slowKeys
                }

                checked: kcm.keyboardFiltersSettings.slowKeysPressBeep
                onToggled: kcm.keyboardFiltersSettings.slowKeysPressBeep = checked
            }
        }
        Kirigami.FormEntry {
            contentItem: QQC2.CheckBox {
                id: slowKeysAcceptBeep

                text: i18nc("@option:check Use system bell when a key is accepted", "when any key is &accepted")

                KCMUtils.SettingStateBinding {
                    configObject: kcm.keyboardFiltersSettings
                    settingName: "SlowKeysAcceptBeep"
                    extraEnabledConditions: kcm.keyboardFiltersSettings.slowKeys
                }

                checked: kcm.keyboardFiltersSettings.slowKeysAcceptBeep
                onToggled: kcm.keyboardFiltersSettings.slowKeysAcceptBeep = checked
            }
        }
        Kirigami.FormEntry {
            contentItem: QQC2.CheckBox {
                id: slowKeysRejectBeep

                text: i18nc("@option:check Use system bell when a key is rejected", "when any key is &rejected")

                KCMUtils.SettingStateBinding {
                    configObject: kcm.keyboardFiltersSettings
                    settingName: "SlowKeysRejectBeep"
                    extraEnabledConditions: kcm.keyboardFiltersSettings.slowKeys
                }

                checked: kcm.keyboardFiltersSettings.slowKeysRejectBeep
                onToggled: kcm.keyboardFiltersSettings.slowKeysRejectBeep = checked
            }
        }
    }
    Kirigami.FormGroup {
        Kirigami.FormEntry {
            title: i18nc("@title:group prefix", "Bounce keys:")
            contentItem: QQC2.CheckBox {
                id: bounceKeys

                text: i18nc("@option:check Bounce keys enable", "Enable");

                KCMUtils.SettingStateBinding {
                    configObject: kcm.keyboardFiltersSettings
                    settingName: "BounceKeys"
                }

                checked: kcm.keyboardFiltersSettings.bounceKeys
                onToggled: kcm.keyboardFiltersSettings.bounceKeys = checked
            }
            trailingItems: [
                Kirigami.ContextualHelpButton {
                    toolTipText: i18nc("@info:tooltip", "Ignore rapid, repeated keypresses of the same key. Useful if you have hand tremors that cause you to press a key multiple times when you only intend to press once.")
                }
            ]
        }

        Kirigami.FormEntry {
            title: i18nc("@label:spinbox Bounce keys delay", "Delay:")
            contentItem: QQC2.SpinBox {
                id: bounceKeysDelay


                KCMUtils.SettingStateBinding {
                    configObject: kcm.keyboardFiltersSettings
                    settingName: "BounceKeysDelay"
                    extraEnabledConditions: kcm.keyboardFiltersSettings.bounceKeys
                }

                from: 5
                to: 10000

                value: kcm.keyboardFiltersSettings.bounceKeysDelay
                onValueModified: kcm.keyboardFiltersSettings.bounceKeysDelay = value

                textFromValue: function(value, locale) {
                    return i18ncp("@label:valuesuffix %1 is bounce keys delay", "%1 ms", "%1 ms", value)
                }
                validator: IntValidatorWithSuffix {
                    bottom: bounceKeysDelay.from
                    top: bounceKeysDelay.to
                }
                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18ncp("@label:valuesuffix short for millisecond(s)", "ms", "ms"), ""))
                }
            }
        }

        Kirigami.FormEntry {
            contentItem: QQC2.CheckBox {
                id: bounceKeysRejectBeep

                text: i18nc("@option:check for bounce keys", "Ring system bell when rejected")

                KCMUtils.SettingStateBinding {
                    configObject: kcm.keyboardFiltersSettings
                    settingName: "BounceKeysRejectBeep"
                    extraEnabledConditions: kcm.keyboardFiltersSettings.bounceKeys
                }

                checked: kcm.keyboardFiltersSettings.bounceKeysRejectBeep
                onToggled: kcm.keyboardFiltersSettings.bounceKeysRejectBeep = checked
            }
        }
    }
}
