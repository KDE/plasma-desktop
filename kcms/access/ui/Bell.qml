/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Dialogs 6.3 as Dialogs
import org.kde.kcmutils as KCM
import org.kde.kquickcontrols 2.0 as KQuickAddons
import org.kde.kirigami 2.3 as Kirigami

Kirigami.FormLayout {

    Dialogs.FileDialog {
        id: fileDialog
        title: i18n("Please choose an audio file")
        nameFilters: [i18nc("Name filters in a file dialog", "Audio Files (*.ogg *.wav)")]
        onAccepted: {
            kcm.bellSettings.customBellFile = fileDialog.selectedFile
        }
    }

    QQC2.CheckBox {
        id: systemBell

        Kirigami.FormData.label: i18n("Audible bell:")
        text: i18nc("Enable the system bell", "Enable")
        KCM.SettingStateBinding {
            configObject: kcm.bellSettings
            settingName: "SystemBell"
        }

        checked: kcm.bellSettings.systemBell
        onToggled: kcm.bellSettings.systemBell = checked
    }
    QQC2.Button {
        text:i18nc("@action:button", "Change Sound…")
        onClicked: KCM.KCMLauncher.openSystemSettings("kcm_notifications", ["--notifyrc", "plasma_workspace", "--event-id", "beep"])
    }


    Item {
        Kirigami.FormData.isSection: true
    }
    QQC2.CheckBox {
        id: visibleBell

        Kirigami.FormData.label: i18n("Visual bell:")
        text: i18nc("Enable visual bell", "Enable")

        KCM.SettingStateBinding {
            configObject: kcm.bellSettings
            settingName: "VisibleBell"
        }

        checked: kcm.bellSettings.visibleBell
        onToggled: kcm.bellSettings.visibleBell = checked
    }

    QQC2.RadioButton {
        id: invertScreen

        text: i18nc("Invert screen on a system bell", "Invert Screen")

        KCM.SettingStateBinding {
            configObject: kcm.bellSettings
            settingName: "InvertScreen"
            extraEnabledConditions: kcm.bellSettings.visibleBell
        }

        checked: kcm.bellSettings.invertScreen
        onToggled: kcm.bellSettings.invertScreen = checked
    }
    RowLayout {
        enabled: kcm.bellSettings.visibleBell

        QQC2.RadioButton {
            id: flashScreen

            text: i18nc("Flash screen on a system bell", "Flash")

            KCM.SettingStateBinding {
                configObject: kcm.bellSettings
                settingName: "InvertScreen"
            }

            checked: !kcm.bellSettings.invertScreen
            onToggled: kcm.bellSettings.invertScreen = !checked
        }
        KQuickAddons.ColorButton {
            text: i18nc("Color of the system bell","Color")
            // avoid to show text outside button
            display: QQC2.AbstractButton.IconOnly

            KCM.SettingStateBinding {
                configObject: kcm.bellSettings
                settingName: "VisibleBellColor"
            }

            color: kcm.bellSettings.visibleBellColor
            onAccepted: color => kcm.bellSettings.visibleBellColor = color
        }
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18nc("Duration of the system bell", "Duration:")

        KCM.SettingStateBinding {
            configObject: kcm.bellSettings
            settingName: "VisibleBellPause"
            extraEnabledConditions: kcm.bellSettings.visibleBell
        }

        value: kcm.bellSettings.visibleBellPause
        onValueModified: kcm.bellSettings.visibleBellPause = value
    }
}
