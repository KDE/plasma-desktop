/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as QQC2
import org.kde.kcmutils as KCM
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

    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Acceleration delay:")

        from: 1
        to: 490

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "AccelerationDelay"
        }

        value: kcm.mouseSettings.accelerationDelay
        onValueChanged: kcm.mouseSettings.accelerationDelay = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Repeat interval:")

        from: 1
        to: 130

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "RepetitionInterval"
        }

        value: kcm.mouseSettings.repetitionInterval
        onValueChanged: kcm.mouseSettings.repetitionInterval = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Acceleration time:")

        from: 1
        to: 100

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "AccelerationTime"
        }

        value: kcm.mouseSettings.accelerationTime
        onValueChanged: kcm.mouseSettings.accelerationTime = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label:  i18n("Maximum speed:")

        from: 1
        to: 100

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "MaxSpeed"
        }

        value: kcm.mouseSettings.maxSpeed
        onValueChanged: kcm.mouseSettings.maxSpeed = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Pointer acceleration:")

        from: -1000
        to: 5000

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "ProfileCurve"
        }

        value: kcm.mouseSettings.profileCurve
        onValueChanged: kcm.mouseSettings.profileCurve = value
    }
}
