/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import org.kde.config as KConfig

import org.kde.plasma.runners.kcm

KCM.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 32
    implicitHeight: Kirigami.Units.gridUnit * 22

    Kirigami.Form {
        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: positionGroup }
            Kirigami.FormEntry {
                title: i18nc("@title:group prefix for radiobutton group", "Position on screen:")
                contentItem: QQC2.RadioButton {
                    onToggled: kcm.krunnerSettings.freeFloating = false
                    checked: !kcm.krunnerSettings.freeFloating
                    QQC2.ButtonGroup.group: positionGroup
                    text: i18nc("@option:check Position on top of screen", "Top")
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    checked: kcm.krunnerSettings.freeFloating
                    onToggled: kcm.krunnerSettings.freeFloating = true
                    QQC2.ButtonGroup.group: positionGroup
                    text: i18nc("@option:check Position in center of screen", "Center")

                    KCM.SettingStateBinding {
                        configObject: kcm.krunnerSettings
                        settingName: "freeFloating"
                    }
                }
            }
        }
        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: historyGroup }
            Kirigami.FormEntry {
                title: i18nc("@title:group prefix for radiobutton group", "History:")
                contentItem: QQC2.RadioButton {
                    checked: kcm.krunnerSettings.historyBehavior === KRunnerSettings.Disabled
                    onToggled: kcm.krunnerSettings.historyBehavior = KRunnerSettings.Disabled
                    text: i18nc("@option:radio History is disabled", "Disabled")
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    checked: kcm.krunnerSettings.historyBehavior === KRunnerSettings.CompletionSuggestion
                    onToggled: kcm.krunnerSettings.historyBehavior = KRunnerSettings.CompletionSuggestion
                    text: i18nc("@option:radio The thing being enabled is search history", "Enable suggestions")
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    checked: kcm.krunnerSettings.historyBehavior === KRunnerSettings.ImmediateCompletion
                    onToggled: kcm.krunnerSettings.historyBehavior = KRunnerSettings.ImmediateCompletion
                    text: i18nc("@option:radio The thing being enabled is search history", "Enable auto-complete")
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.Button {
                    id: clearHistoryButton
                    enabled: kcm.krunnerSettings.historyBehavior !== KRunnerSettings.Disabled && kcm.historyKeys.length > 0

                    icon.name: Application.layoutDirection === Qt.LeftToRight ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
                    text: i18nc("@action:button", "Clear History…")

                    checked: activityMenu.visible

                    // NOTE: Use onReleased to avoid race condition
                    onReleased: {
                        if (!activityList.model) {
                            activityList.model = Qt.createQmlObject("import org.kde.activities 0.1; ActivityModel {}", root); // Lazy load
                        }

                        if (activityMenu.visible) {
                            activityMenu.close();
                        } else {
                            activityMenu.popup(x, y + implicitHeight);
                        }
                    }
                }

                QQC2.Menu {
                    id: activityMenu

                    QQC2.MenuItem {
                        icon.name: clearHistoryButton.icon.name
                        text: i18nc("@item:inmenu delete krunner history for all activities", "For all activities")

                        onTriggered: kcm.deleteAllHistory()
                    }

                    Repeater {
                        id: activityList

                        QQC2.MenuItem {
                            enabled: kcm.historyKeys.includes(model.id)

                            icon.source: model.iconSource
                            text: i18nc("@item:inmenu delete krunner history for this activity", "For activity \"%1\"", model.name)

                            onTriggered: kcm.deleteHistoryGroup(model.id)
                        }
                    }
                }
            }
        }
        Kirigami.FormGroup {
            visible: kcm.doesShowPluginButton
            Kirigami.FormAction {
                action: Kirigami.Action {
                    enabled: KConfig.KAuthorized.authorizeControlModule("kcm_krunnersettings")
                    text: i18nc("@action:button", "Configure Enabled Search Plugins…")
                    icon.name: "plasma-search"

                    onTriggered: KCM.KCMLauncher.openSystemSettings("kcm_plasmasearch")
                }
            }
        }
    }
}
