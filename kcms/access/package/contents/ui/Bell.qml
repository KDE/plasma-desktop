/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Dialogs 1.3 as Dialogs

import org.kde.kcm 1.3 as KCM
import org.kde.kquickcontrols 2.0 as KQuickAddons
import org.kde.kirigami 2.3 as Kirigami

Kirigami.FormLayout {

    Dialogs.FileDialog {
        id: fileDialog
        title: i18n("Please choose an audio file")
        folder: shortcuts.home
        nameFilters: [i18n("Audio Files (*.mp3 *.ogg *.wav)")]
        onAccepted: {
            kcm.bellSettings.customBellFile = fileDialog.fileUrls[0]
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

    RowLayout {
        Kirigami.FormData.label: i18nc("Defines if the system will use a sound system bell", "Custom sound:")
        spacing: 0

        QQC2.CheckBox {
            id: customBell
            Layout.alignment: Qt.AlignVCenter

            KCM.SettingStateBinding {
                configObject: kcm.bellSettings
                settingName: "CustomBell"
                extraEnabledConditions: kcm.bellSettings.systemBell
            }

            checked: kcm.bellSettings.customBell
            onToggled: kcm.bellSettings.customBell= checked
        }

        QQC2.TextField {
            id: textEdit

            text: kcm.bellSettings.customBellFile

            KCM.SettingStateBinding {
                configObject: kcm.bellSettings
                settingName: "CustomBellFile"
                extraEnabledConditions: kcm.bellSettings.customBell
            }

            onEditingFinished: kcm.bellSettings.customBellFile = textEdit.text
        }
        QQC2.Button {
            icon.name: "folder"
            QQC2.ToolTip.visible: down
            QQC2.ToolTip.text: i18n("Search audio file for the system bell")
            Accessible.name: i18n("Button search audio file")
            KCM.SettingStateBinding {
                configObject: kcm.bellSettings
                settingName: "CustomBellFile"
                extraEnabledConditions: kcm.bellSettings.customBell
            }

            onClicked: fileDialog.open()
        }
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
            KCM.SettingStateBinding {
                configObject: kcm.bellSettings
                settingName: "VisibleBellColor"
            }

            color: kcm.bellSettings.visibleBellColor
            onAccepted: kcm.bellSettings.visibleBellColor = color
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
