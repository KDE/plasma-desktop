/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs as Dialogs
import org.kde.kcmutils as KCMUtils
import org.kde.kquickcontrols as KQuickAddons
import org.kde.kirigami as Kirigami

Kirigami.Form {
    Dialogs.FileDialog {
        id: fileDialog
        title: i18nc("@title:window dialog","Please choose an audio file")
        nameFilters: [i18nc("Name filters in a file dialog. Do not translate `(*.ogg *.oga *.wav)`",
                            "ogg, oga, and wav audio files (*.ogg *.oga *.wav)")]
        onAccepted: {
            kcm.bellSettings.customBellFile = fileDialog.selectedFile
        }
    }

    Kirigami.FormGroup {
        Kirigami.FormEntry {
            title: i18nc("@label prefix for checkbox", "Audible bell:")
            contentItem: QQC2.CheckBox {
                id: systemBell

                text: i18nc("Enable the system bell", "Enable")
                KCMUtils.SettingStateBinding {
                    configObject: kcm.bellSettings
                    settingName: "SystemBell"
                }

                checked: kcm.bellSettings.systemBell
                onToggled: kcm.bellSettings.systemBell = checked
            }
        }

        Kirigami.FormEntry {
            title: i18nc("Defines if the system will use a sound system bell", "Custom sound:")
            contentItem: QQC2.CheckBox {
                id: customBell

                KCMUtils.SettingStateBinding {
                    configObject: kcm.bellSettings
                    settingName: "CustomBell"
                    extraEnabledConditions: kcm.bellSettings.systemBell
                }

                checked: kcm.bellSettings.customBell
                onToggled: kcm.bellSettings.customBell= checked
            }
            trailingItems: [
                QQC2.TextField {
                    id: textEdit

                    text: kcm.bellSettings.customBellFile

                    KCMUtils.SettingStateBinding {
                        configObject: kcm.bellSettings
                        settingName: "CustomBellFile"
                        extraEnabledConditions: kcm.bellSettings.customBell
                    }

                    onEditingFinished: kcm.bellSettings.customBellFile = textEdit.text
                },
                QQC2.Button {
                    icon.name: "folder"
                    QQC2.ToolTip.visible: down
                    QQC2.ToolTip.text: i18nc("@action:button icononly tooltip", "Choose audio file for the system bell")
                    Accessible.name: i18nc("@action button accessible only", "Choose audio file")
                    Accessible.description: i18nc("@info:usagetip accessible description for button", "Opens dialog")
                    KCMUtils.SettingStateBinding {
                        configObject: kcm.bellSettings
                        settingName: "CustomBellFile"
                        extraEnabledConditions: kcm.bellSettings.customBell
                    }

                    onClicked: fileDialog.open()
                }
            ]
        }
    }

    Kirigami.FormGroup {
        Kirigami.FormEntry {
            title: i18nc("@label prefix for checkbox", "Visual bell:")
            contentItem: QQC2.CheckBox {
                id: visibleBell

                text: i18nc("@option:check Enable visual bell", "Enable")

                KCMUtils.SettingStateBinding {
                    configObject: kcm.bellSettings
                    settingName: "VisibleBell"
                }

                checked: kcm.bellSettings.visibleBell
                onToggled: kcm.bellSettings.visibleBell = checked
            }
        }

        Kirigami.FormEntry {
            contentItem: QQC2.RadioButton {
                id: invertScreen

                text: i18nc("@option:radio Invert screen colors when a system bell is rung", "Invert screen colors")

                KCMUtils.SettingStateBinding {
                    configObject: kcm.bellSettings
                    settingName: "InvertScreen"
                    extraEnabledConditions: kcm.bellSettings.visibleBell
                }

                checked: kcm.bellSettings.invertScreen
                onToggled: kcm.bellSettings.invertScreen = checked
            }
        }

        Kirigami.FormEntry {
            enabled: kcm.bellSettings.visibleBell
            contentItem: QQC2.RadioButton {
                id: flashScreen

                text: i18nc("@option:radio Flash screen when a system bell is rung", "Flash screen")

                KCMUtils.SettingStateBinding {
                    configObject: kcm.bellSettings
                    settingName: "InvertScreen"
                }

                checked: !kcm.bellSettings.invertScreen
                onToggled: kcm.bellSettings.invertScreen = !checked
            }
            trailingItems: [
                KQuickAddons.ColorButton {
                    text: i18nc("Color of the system bell","Color")
                    // avoid to show text outside button
                    display: QQC2.AbstractButton.IconOnly

                    KCMUtils.SettingStateBinding {
                        configObject: kcm.bellSettings
                        settingName: "VisibleBellColor"
                    }

                    color: kcm.bellSettings.visibleBellColor
                    onAccepted: color => kcm.bellSettings.visibleBellColor = color
                }
            ]
        }

        Kirigami.FormEntry {
            title: i18nc("Duration of the system bell", "Duration:")
            contentItem: QQC2.SpinBox {

                from: 100
                to: 2000

                KCMUtils.SettingStateBinding {
                    configObject: kcm.bellSettings
                    settingName: "VisibleBellPause"
                    extraEnabledConditions: kcm.bellSettings.visibleBell
                }

                value: kcm.bellSettings.visibleBellPause
                onValueModified: kcm.bellSettings.visibleBellPause = value

                textFromValue: function(value, locale) {
                    return i18ncp("@label:valuesuffix %1 is visible bell duration", "%1 ms", "%1 ms", value)
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18ncp("@label:valuesuffix short for millisecond(s)", "ms", "ms"), ""))
                }
            }
        }
    }
}
