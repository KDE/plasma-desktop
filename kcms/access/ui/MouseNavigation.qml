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
            title: i18nc("@option:check", "Use number pad to move pointer:")
            contentItem: QQC2.CheckBox {
                text: i18nc("@option:check Enable mouse navigation", "Enable")

                KCMUtils.SettingStateBinding {
                    configObject: kcm.mouseSettings
                    settingName: "MouseKeys"
                }

                checked: kcm.mouseSettings.mouseKeys
                onToggled: kcm.mouseSettings.mouseKeys = checked
            }
            trailingItems: [
                Kirigami.ContextualHelpButton {
                    toolTipText: xi18nc("@info:tooltip", "The numpad key <shortcut>5</shortcut> functions as a mouse click, toggled by <shortcut>/</shortcut> for middle click, <shortcut>*</shortcut> for right click and <shortcut>NumLock</shortcut> for left click. The keys <shortcut>2</shortcut>, <shortcut>4</shortcut>, <shortcut>6</shortcut>, and <shortcut>8</shortcut> allow for cardinal movement (down, left, right, and up). The keys <shortcut>1</shortcut>, <shortcut>3</shortcut>, <shortcut>7</shortcut>, and <shortcut>9</shortcut> allow for diagonal movement.")
                }
            ]
        }

        Kirigami.FormEntry {
            title: i18nc("@label:spinbox", "Acceleration delay:")
            contentItem: QQC2.SpinBox {
                id: accelerationDelay

                from: 1
                to: 490

                KCMUtils.SettingStateBinding {
                    configObject: kcm.mouseSettings
                    settingName: "AccelerationDelay"
                }

                value: kcm.mouseSettings.accelerationDelay
                onValueChanged: kcm.mouseSettings.accelerationDelay = value

                textFromValue: function(value, locale) {
                    return i18ncp("@label:valuesuffix acceleration delay spinbox %1 is value", "%1 ms", "%1 ms", value)
                }
                validator: IntValidatorWithSuffix {
                    bottom: accelerationDelay.from
                    top: accelerationDelay.to
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18ncp("short for millisecond(s)", "ms", "ms"), ""))
                }
            }
        }
        Kirigami.FormEntry {
            title: i18nc("@label:spinbox", "Repeat interval:")
            contentItem: QQC2.SpinBox {
                id: repeatInterval

                from: 1
                to: 130

                KCMUtils.SettingStateBinding {
                    configObject: kcm.mouseSettings
                    settingName: "RepetitionInterval"
                }

                value: kcm.mouseSettings.repetitionInterval
                onValueChanged: kcm.mouseSettings.repetitionInterval = value

                validator: IntValidatorWithSuffix {
                    bottom: repeatInterval.from
                    top: repeatInterval.to
                }
                textFromValue: function(value, locale) {
                    return i18ncp("@label:valuesuffix repeat interval spinbox %1 is value", "%1 ms", "%1 ms", value)
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18ncp("short for millisecond(s)", "ms", "ms"), ""))
                }
            }
        }
        Kirigami.FormEntry {
            title: i18nc("@label:spinbox", "Acceleration time:")
            contentItem: QQC2.SpinBox {
                id: accelerationTime

                from: 1
                to: 100

                KCMUtils.SettingStateBinding {
                    configObject: kcm.mouseSettings
                    settingName: "AccelerationTime"
                }

                value: kcm.mouseSettings.accelerationTime
                onValueChanged: kcm.mouseSettings.accelerationTime = value
                validator: IntValidatorWithSuffix {
                    bottom: accelerationTime.from
                    top: accelerationTime.to
                }
                textFromValue: function(value, locale) {
                    return i18ncp("@label:valuesuffix acceleration time spinbox %1 is value", "%1 ms", "%1 ms", value)
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18ncp("short for millisecond(s)", "ms", "ms"), ""))
                }
            }
        }
        Kirigami.FormEntry {
            title: i18nc("@label:spinbox", "Maximum speed:")
            contentItem: QQC2.SpinBox {

                from: 1
                to: 100

                KCMUtils.SettingStateBinding {
                    configObject: kcm.mouseSettings
                    settingName: "MaxSpeed"
                }

                value: kcm.mouseSettings.maxSpeed
                onValueChanged: kcm.mouseSettings.maxSpeed = value
            }
        }
        Kirigami.FormEntry {
            title: i18nc("@label:spinbox", "Pointer acceleration:")
            contentItem: QQC2.SpinBox {

                from: -1000
                to: 5000

                KCMUtils.SettingStateBinding {
                    configObject: kcm.mouseSettings
                    settingName: "ProfileCurve"
                }

                value: kcm.mouseSettings.profileCurve
                onValueChanged: kcm.mouseSettings.profileCurve = value

            }
        }
    }
}
