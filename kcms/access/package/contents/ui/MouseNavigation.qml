/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as QQC2
import org.kde.kcm 1.3 as KCM
import org.kde.kirigami 2.3 as Kirigami

Kirigami.FormLayout {
    QQC2.CheckBox {
        id: mouseKeys

        Kirigami.FormData.label:  i18n("Use number pad to move cursor:")
        text: i18n("Enable")

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "MouseKeys"
        }

        checked: kcm.mouseSettings.mouseKeys
        onToggled: kcm.mouseSettings.mouseKeys = checked
    }
    QQC2.CheckBox {
        Kirigami.FormData.label: i18n("When a gesture is used:")
        text: i18n("Display a confirmation dialog")

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "GestureConfirmation"
        }

        checked: kcm.mouseSettings.gestureConfirmation
        onToggled: kcm.mouseSettings.gestureConfirmation = checked
    }
    QQC2.CheckBox {
        text: i18n("Ring the System Bell")

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "Gestures"
        }

        checked: kcm.mouseSettings.gestures
        onToggled: kcm.mouseSettings.gestures = checked
    }
    QQC2.CheckBox {
        text: i18n("Show a notification")

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "KeyboardNotifyAccess"
        }

        checked: kcm.mouseSettings.keyboardNotifyAccess
        onToggled: kcm.mouseSettings.keyboardNotifyAccess = checked
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Acceleration delay:")

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "AccelerationDelay"
        }

        value: kcm.mouseSettings.accelerationDelay
        onValueChanged: kcm.mouseSettings.accelerationDelay = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Repeat interval:")

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "RepetitionInterval"
        }

        value: kcm.mouseSettings.repetitionInterval
        onValueChanged: kcm.mouseSettings.repetitionInterval = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Acceleration time:")

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "AccelerationTime"
        }

        value: kcm.mouseSettings.accelerationTime
        onValueChanged: kcm.mouseSettings.accelerationTime = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label:  i18n("Maximum speed:")

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "MaxSpeed"
        }

        value: kcm.mouseSettings.maxSpeed
        onValueChanged: kcm.mouseSettings.maxSpeed = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Acceleration profile:")

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "ProfileCurve"
        }

        value: kcm.mouseSettings.profileCurve
        onValueChanged: kcm.mouseSettings.profileCurve = value
    }
}
