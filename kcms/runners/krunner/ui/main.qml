/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.19 as Kirigami

import org.kde.kcmutils as KCM
import org.kde.config as KConfig

KCM.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 32
    implicitHeight: Kirigami.Units.gridUnit * 18

    Kirigami.FormLayout {
        width: parent.width
        anchors.top: parent.top

        QQC2.ButtonGroup {
            id: positionGroup
        }

        QQC2.RadioButton {
            Kirigami.FormData.label: i18n("Position on screen:")
            onToggled: kcm.krunnerSettings.freeFloating = false
            checked: !kcm.krunnerSettings.freeFloating
            QQC2.ButtonGroup.group: positionGroup
            text: i18n("Top")
        }

        QQC2.RadioButton {
            checked: kcm.krunnerSettings.freeFloating
            onToggled: kcm.krunnerSettings.freeFloating = true
            QQC2.ButtonGroup.group: positionGroup
            text: i18n("Center")

            KCM.SettingStateBinding {
                configObject: kcm.krunnerSettings
                settingName: "freeFloating"
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        QQC2.CheckBox {
            Kirigami.FormData.label: i18n("Activation:")
            checked: kcm.krunnerSettings.activateWhenTypingOnDesktop
            onToggled: kcm.krunnerSettings.activateWhenTypingOnDesktop = checked
            text: i18nc("@option:check", "Activate when pressing any key on the desktop")

            KCM.SettingStateBinding {
                configObject: kcm.krunnerSettings
                settingName: "activateWhenTypingOnDesktop"
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        QQC2.CheckBox {
            Kirigami.FormData.label: i18n("History:")
            checked: kcm.krunnerSettings.historyEnabled
            onToggled: kcm.krunnerSettings.historyEnabled = checked
            text: i18nc("@option:check", "Remember past searches")

            KCM.SettingStateBinding {
                configObject: kcm.krunnerSettings
                settingName: "historyEnabled"
            }
        }

        QQC2.CheckBox {
            checked: kcm.krunnerSettings.retainPriorSearch
            onToggled: kcm.krunnerSettings.retainPriorSearch = checked
            text: i18nc("@option:check", "Retain last search when re-opening")

            KCM.SettingStateBinding {
                configObject: kcm.krunnerSettings
                settingName: "retainPriorSearch"
            }
        }

        QQC2.CheckBox {
            checked: kcm.krunnerSettings.activityAware
            onToggled: kcm.krunnerSettings.activityAware = checked
            text: i18nc("@option:check", "Activity-aware (last search and history)")

            KCM.SettingStateBinding {
                configObject: kcm.krunnerSettings
                settingName: "activityAware"
            }
        }

        QQC2.Button {
            id: clearHistoryButton
            enabled: kcm.krunnerSettings.historyEnabled && kcm.historyKeys.length > 0

            icon.name: Qt.application.layoutDirection === Qt.LeftToRight ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
            text: kcm.hasSingleHistory ?
                    (!kcm.krunnerSettings.activityAware || kcm.activityCount < 2 ?
                        i18n("Clear History")
                      : i18nc("@action:button %1 activity name", "Clear History for Activity \"%1\"", kcm.singleActivityName))
                  : i18n("Clear History…")

            checkable: !kcm.hasSingleHistory
            checked: activityMenu.visible

            // NOTE: Use onReleased to avoid race condition
            onReleased: {
                if (kcm.hasSingleHistory) {
                    kcm.deleteAllHistory();
                    return;
                }

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

                    icon.name: kcm.iconNameForActivity(model.id)
                    icon.source: model.iconSource
                    text: i18nc("@item:inmenu delete krunner history for this activity", "For activity \"%1\"", model.name)

                    onTriggered: kcm.deleteHistoryGroup(model.id)
                }
            }
        }

        Item {
            visible: pluginButton.visible
            Kirigami.FormData.isSection: false
        }

        Loader {
            id: pluginButton
            active: kcm.doesShowPluginButton
            visible: active

            Kirigami.FormData.label: i18nc("@label", "Plugins:")

            sourceComponent: QQC2.Button {
                enabled: KConfig.KAuthorized.authorizeControlModule("kcm_krunnersettings")
                text: i18nc("@action:button", "Configure Enabled Search Plugins…")
                icon.name: "settings-configure"

                onClicked: KCM.KCMLauncher.openSystemSettings("kcm_plasmasearch")
            }
        }
    }
}
