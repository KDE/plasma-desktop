/*
 *   SPDX-FileCopyrightText: 2026 Sebastian Sauer <dipesh@gmx.de>
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols as KQControls
import org.kde.plasma.access.kcm

Kirigami.Form {
    id: rootForm

    Kirigami.FormGroup {
        Kirigami.FormEntry {
            title: i18nc("@label", "Click when pointer remains still:")
            contentItem: QQC2.CheckBox {
                text: i18nc("@option check, Enable dwell clicker effect", "Enable")

                KCMUtils.SettingStateBinding {
                    configObject: kcm.dwellClickerSettings
                    settingName: "DwellClicker"
                }

                checked: kcm.dwellClickerSettings.dwellClicker
                onToggled: kcm.dwellClickerSettings.dwellClicker = checked
            }
            trailingItems: [
                Kirigami.ContextualHelpButton {
                    toolTipText: i18nc("@info:tooltip", "Automatically click when pointer remains still for a set amount of time.")
                }
            ]
        }

        Kirigami.FormEntry {
            title: i18nc("@label:spinbox Slow keys Delay", "Motion Threshold:")
            contentItem: QQC2.SpinBox {
                id: motionThresholdItem

                KCMUtils.SettingStateBinding {
                    configObject: kcm.dwellClickerSettings
                    settingName: "DwellClickerMotionThreshold"
                    extraEnabledConditions: kcm.dwellClickerSettings.dwellClicker
                }

                from: 0
                to: 100
                value: kcm.dwellClickerSettings.dwellClickerMotionThreshold
                onValueModified: kcm.dwellClickerSettings.dwellClickerMotionThreshold = value

                textFromValue: function(value, locale) {
                    return i18ncp("@label:valuesuffix %1 is the motion threshold", "%1 px", "%1 px", value)
                }

                validator: IntValidatorWithSuffix {
                    bottom: motionThresholdItem.from
                    top: motionThresholdItem.to
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18ncp("@label:valuesuffix short for pixel(s)", "px", "px"), ""))
                }
            }
        }

        Kirigami.FormEntry {
            title: i18nc("@label:spinbox Dwell clicker delay time", "Delay Time:")
            contentItem: QQC2.SpinBox {
                id: delayTimeItem

                KCMUtils.SettingStateBinding {
                    configObject: kcm.dwellClickerSettings
                    settingName: "DwellClickerDelayTime"
                    extraEnabledConditions: kcm.dwellClickerSettings.dwellClicker
                }

                from: 0
                to: 30000
                value: kcm.dwellClickerSettings.dwellClickerDelayTime
                onValueModified: kcm.dwellClickerSettings.dwellClickerDelayTime = value

                textFromValue: function(value, locale) {
                    return i18ncp("@label:valuesuffix %1 is the delay time", "%1 ms", "%1 ms", value)
                }

                validator: IntValidatorWithSuffix {
                    bottom: delayTimeItem.from
                    top: delayTimeItem.to
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18ncp("@label:valuesuffix short for millisecond(s)", "ms", "ms"), ""))
                }
            }
        }

        Kirigami.FormEntry {
            title: i18nc("@label:spinbox Dwell clicker dwell time", "Dwell Time:")
            contentItem: QQC2.SpinBox {
                id: dwellTimeItem

                KCMUtils.SettingStateBinding {
                    configObject: kcm.dwellClickerSettings
                    settingName: "DwellClickerDwellTime"
                    extraEnabledConditions: kcm.dwellClickerSettings.dwellClicker
                }

                from: 0
                to: 30000
                value: kcm.dwellClickerSettings.dwellClickerDwellTime
                onValueModified: kcm.dwellClickerSettings.dwellClickerDwellTime = value

                textFromValue: function(value, locale) {
                    return i18ncp("@label:valuesuffix %1 is the dwell time", "%1 ms", "%1 ms", value)
                }

                validator: IntValidatorWithSuffix {
                    bottom: dwellTimeItem.from
                    top: dwellTimeItem.to
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18ncp("@label:valuesuffix short for millisecond(s)", "ms", "ms"), ""))
                }
            }
        }

        Kirigami.FormEntry {
            title: i18nc("@label:button Dwell clicker dwell color", "Dwell Color:")
            contentItem: KQControls.ColorButton {
                id: colorButton

                color: kcm.dwellClickerSettings.dwellClickerDwellColor
                onColorChanged: {
                    kcm.dwellClickerSettings.dwellClickerDwellColor = colorButton.color
                }

                KCMUtils.SettingStateBinding {
                    configObject: kcm.dwellClickerSettings
                    settingName: "DwellClickerDwellColor"
                    extraEnabledConditions: kcm.dwellClickerSettings.dwellClicker
                }
            }
        }

    }
}
