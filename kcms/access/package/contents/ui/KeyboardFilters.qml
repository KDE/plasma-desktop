/*
 * Copyright 2018 Tomaz Canabrava <tcanabrava@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
            configObject: kcm.keyboardSettings
            settingName: "SlowKeys"
        }

        checked: kcm.keyboardSettings.slowKeys
        onToggled: kcm.keyboardSettings.slowKeys = checked
    }

    QQC2.SpinBox {
        id: slowKayDelay

        Kirigami.FormData.label: i18nc("Slow keys Delay", "Delay:")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "SlowKeysDelay"
            extraEnabledConditions: kcm.keyboardSettings.slowKeys
        }

        onValueChanged: kcm.keyboardSettings.slowKeysDelay = value
    }
    Item {
        Kirigami.FormData.isSection: true
    }
    QQC2.CheckBox {
        id: slowKeysPressBeep

        Kirigami.FormData.label: i18n("Ring system bell:")
        text: i18nc("Use system bell when a key is pressed", "&when any key is pressed")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "SlowKeysPressBeep"
            extraEnabledConditions: kcm.keyboardSettings.slowKeys
        }

        checked: kcm.keyboardSettings.slowKeysPressBeep
        onToggled: kcm.keyboardSettings.slowKeysPressBeep = checked
    }
    QQC2.CheckBox {
        id: slowKeysAcceptBeep

        text: i18nc("Use system bell when a key is accepted", "&when any key is accepted")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "SlowKeysAcceptBeep"
            extraEnabledConditions: kcm.keyboardSettings.slowKeys
        }

        checked: kcm.keyboardSettings.slowKeysAcceptBeep
        onToggled: kcm.keyboardSettings.slowKeysAcceptBeep = checked
    }
    QQC2.CheckBox {
        id: slowKeysRejectBeep

        text: i18nc("Use system bell when a key is rejected", "&when any key is rejected")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "SlowKeysRejectBeep"
            extraEnabledConditions: kcm.keyboardSettings.slowKeys
        }

        checked: kcm.keyboardSettings.slowKeysRejectBeep
        onToggled: kcm.keyboardSettings.slowKeysRejectBeep = checked
    }
    Item {
        Kirigami.FormData.isSection: true
    }
    QQC2.CheckBox {
        id: bounceKeys

        Kirigami.FormData.label: i18n("Bounce keys:")
        text: i18nc("Bounce keys enable", "Enable");

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "BounceKeys"
        }

        checked: kcm.keyboardSettings.bounceKeys
        onToggled: kcm.keyboardSettings.bounceKeys = checked
    }

    QQC2.SpinBox {
        id: bounceKeysDelay

        Kirigami.FormData.label: i18nc("Bounce keys delay", "Delay:")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "BounceKeysDelay"
            extraEnabledConditions: kcm.keyboardSettings.bounceKeys
        }

        onValueChanged: kcm.keyboardSettings.bounceKeysDelay = value
    }

    QQC2.CheckBox {
        id: bounceKeysRejectBeep

        text: i18n("Ring system bell when rejected")

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "BounceKeysRejectBeep"
            extraEnabledConditions: kcm.keyboardSettings.bounceKeys
        }

        checked: kcm.keyboardSettings.bounceKeysRejectBeep
        onToggled: kcm.keyboardSettings.bounceKeysRejectBeep = checked
    }
}
