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
        id: slowKeys

        Kirigami.FormData.label: i18n("Slow keys:")
        text: i18nc("Enable slow keys", "Enable")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "SlowKeys"
        }

        checked: kcm.keyboardFiltersSettings.slowKeys
        onToggled: kcm.keyboardFiltersSettings.slowKeys = checked
    }

    QQC2.SpinBox {
        id: slowKayDelay

        Kirigami.FormData.label: i18nc("Slow keys Delay", "Delay:")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "SlowKeysDelay"
            extraEnabledConditions: kcm.keyboardFiltersSettings.slowKeys
        }

        value: kcm.keyboardFiltersSettings.slowKeysDelay
        onValueChanged: kcm.keyboardFiltersSettings.slowKeysDelay = value
    }
    Item {
        Kirigami.FormData.isSection: true
    }
    QQC2.CheckBox {
        id: slowKeysPressBeep

        Kirigami.FormData.label: i18n("Ring system bell:")
        text: i18nc("Use system bell when a key is pressed", "&when any key is pressed")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "SlowKeysPressBeep"
            extraEnabledConditions: kcm.keyboardFiltersSettings.slowKeys
        }

        checked: kcm.keyboardFiltersSettings.slowKeysPressBeep
        onToggled: kcm.keyboardFiltersSettings.slowKeysPressBeep = checked
    }
    QQC2.CheckBox {
        id: slowKeysAcceptBeep

        text: i18nc("Use system bell when a key is accepted", "&when any key is accepted")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "SlowKeysAcceptBeep"
            extraEnabledConditions: kcm.keyboardFiltersSettings.slowKeys
        }

        checked: kcm.keyboardFiltersSettings.slowKeysAcceptBeep
        onToggled: kcm.keyboardFiltersSettings.slowKeysAcceptBeep = checked
    }
    QQC2.CheckBox {
        id: slowKeysRejectBeep

        text: i18nc("Use system bell when a key is rejected", "&when any key is rejected")

        KCM.SettingStateBinding {
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
    QQC2.CheckBox {
        id: bounceKeys

        Kirigami.FormData.label: i18n("Bounce keys:")
        text: i18nc("Bounce keys enable", "Enable");

        KCM.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "BounceKeys"
        }

        checked: kcm.keyboardFiltersSettings.bounceKeys
        onToggled: kcm.keyboardFiltersSettings.bounceKeys = checked
    }

    QQC2.SpinBox {
        id: bounceKeysDelay

        Kirigami.FormData.label: i18nc("Bounce keys delay", "Delay:")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "BounceKeysDelay"
            extraEnabledConditions: kcm.keyboardFiltersSettings.bounceKeys
        }

        value: kcm.keyboardFiltersSettings.bounceKeysDelay
        onValueChanged: kcm.keyboardFiltersSettings.bounceKeysDelay = value
    }

    QQC2.CheckBox {
        id: bounceKeysRejectBeep

        text: i18n("Ring system bell when rejected")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardFiltersSettings
            settingName: "BounceKeysRejectBeep"
            extraEnabledConditions: kcm.keyboardFiltersSettings.bounceKeys
        }

        checked: kcm.keyboardFiltersSettings.bounceKeysRejectBeep
        onToggled: kcm.keyboardFiltersSettings.bounceKeysRejectBeep = checked
    }
}
