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
        Kirigami.FormData.label:  i18nc("@option:check", "Use number pad to move cursor:")
        Kirigami.FormData.buddyFor: mouseKeys
        QQC2.CheckBox {
            id: mouseKeys

            text: i18nc("@option:check Enable mouse navigation", "Enable")

            KCMUtils.SettingStateBinding {
                configObject: kcm.mouseSettings
                settingName: "MouseKeys"
            }

            checked: kcm.mouseSettings.mouseKeys
            onToggled: kcm.mouseSettings.mouseKeys = checked
        }
        Kirigami.ContextualHelpButton {
            toolTipText: xi18nc("@info:tooltip", "The numpad key <shortcut>5</shortcut> functions as a mouse click. The keys <shortcut>2</shortcut>, <shortcut>4</shortcut>, <shortcut>6</shortcut>, and <shortcut>8</shortcut> allow for cardinal movement (down, left, right, and up). The keys <shortcut>1</shortcut>, <shortcut>3</shortcut>, <shortcut>7</shortcut>, and <shortcut>9</shortcut> allow for diagonal movement.")
        }
    }

    QQC2.SpinBox {
        Kirigami.FormData.label: i18nc("@label:spinbox", "Acceleration delay:")

        from: 1
        to: 490

        KCMUtils.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "AccelerationDelay"
        }

        value: kcm.mouseSettings.accelerationDelay
        onValueChanged: kcm.mouseSettings.accelerationDelay = value

        textFromValue: function(value, locale) {
            return i18np("%1 ms", "%1 ms", value)
        }

        valueFromText: (text, locale) => {
            return Number.fromLocaleString(locale, text.replace(i18ncp("short for millisecond(s)", "ms", "ms"), ""))
        }
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18nc("@label:spinbox", "Repeat interval:")

        from: 1
        to: 130

        KCMUtils.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "RepetitionInterval"
        }

        value: kcm.mouseSettings.repetitionInterval
        onValueChanged: kcm.mouseSettings.repetitionInterval = value

        textFromValue: function(value, locale) {
            return i18np("%1 ms", "%1 ms", value)
        }

        valueFromText: (text, locale) => {
            return Number.fromLocaleString(locale, text.replace(i18ncp("short for millisecond(s)", "ms", "ms"), ""))
        }
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18nc("@label:spinbox", "Acceleration time:")

        from: 1
        to: 100

        KCMUtils.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "AccelerationTime"
        }

        value: kcm.mouseSettings.accelerationTime
        onValueChanged: kcm.mouseSettings.accelerationTime = value

        textFromValue: function(value, locale) {
            return i18np("%1 ms", "%1 ms", value)
        }

        valueFromText: (text, locale) => {
            return Number.fromLocaleString(locale, text.replace(i18ncp("short for millisecond(s)", "ms", "ms"), ""))
        }
    }
    QQC2.SpinBox {
        Kirigami.FormData.label:  i18nc("@label:spinbox", "Maximum speed:")

        from: 1
        to: 100

        KCMUtils.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "MaxSpeed"
        }

        value: kcm.mouseSettings.maxSpeed
        onValueChanged: kcm.mouseSettings.maxSpeed = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18nc("@label:spinbox", "Pointer acceleration:")

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
