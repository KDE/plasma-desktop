/*
    SPDX-FileCopyrightText: 2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

import org.kde.plasma.workspace.dbus as DBus

KCMUtils.SimpleKCM {
    property alias cfg_groupingStrategy: groupingStrategy.currentIndex
    property alias cfg_groupedTaskVisualization: groupedTaskVisualization.currentIndex
    property alias cfg_groupPopups: groupPopups.checked
    property alias cfg_onlyGroupWhenFull: onlyGroupWhenFull.checked
    property alias cfg_sortingStrategy: sortingStrategy.currentIndex
    property alias cfg_separateLaunchers: separateLaunchers.checked
    property alias cfg_hideLauncherOnStart: hideLauncherOnStart.checked
    property alias cfg_middleClickAction: middleClickAction.currentIndex
    property alias cfg_wheelEnabled: wheelEnabled.checked
    property alias cfg_wheelSkipMinimized: wheelSkipMinimized.checked
    property alias cfg_showOnlyCurrentScreen: showOnlyCurrentScreen.checked
    property alias cfg_showOnlyCurrentDesktop: showOnlyCurrentDesktop.checked
    property alias cfg_showOnlyCurrentActivity: showOnlyCurrentActivity.checked
    property alias cfg_showOnlyMinimized: showOnlyMinimized.checked
    property alias cfg_minimizeActiveTaskOnClick: minimizeActive.checked
    property alias cfg_unhideOnAttention: unhideOnAttention.checked
    property alias cfg_reverseMode: reverseMode.checked

    DBus.DBusServiceWatcher {
        id: effectWatcher
        busType: DBus.BusType.Session
        watchedService: "org.kde.KWin.Effect.WindowView1"
    }

    Kirigami.FormLayout {
        anchors.left: parent.left
        anchors.right: parent.right

        QQC2.ComboBox {
            id: groupingStrategy
            Kirigami.FormData.label: i18n("Group:")
            Layout.fillWidth: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 14
            model: [i18n("Do not group"), i18n("By program name")]
        }

        QQC2.ComboBox {
            id: groupedTaskVisualization
            Kirigami.FormData.label: i18n("Clicking grouped task:")
            Layout.fillWidth: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 14

            enabled: groupingStrategy.currentIndex !== 0

            model: [
                i18nc("Completes the sentence 'Clicking grouped task cycles through tasks' ", "Cycles through tasks"),
                i18nc("Completes the sentence 'Clicking grouped task shows tooltip window thumbnails' ", "Shows small window previews"),
                i18nc("Completes the sentence 'Clicking grouped task shows windows side by side' ", "Shows large window previews"),
                i18nc("Completes the sentence 'Clicking grouped task shows textual list' ", "Shows textual list"),
            ]

            Accessible.name: currentText
            Accessible.onPressAction: currentIndex = currentIndex === count - 1 ? 0 : (currentIndex + 1)
        }
        // "You asked for Window View but Window View is not available" message
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: groupedTaskVisualization.currentIndex === 2 && !effectWatcher.registered
            type: Kirigami.MessageType.Warning
            text: i18n("The compositor does not support displaying windows side by side, so a textual list will be displayed instead.")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: groupPopups
            visible: (Plasmoid.pluginName !== "org.kde.plasma.icontasks")
            text: i18n("Combine into single button")
            enabled: groupingStrategy.currentIndex > 0
        }

        QQC2.CheckBox {
            id: onlyGroupWhenFull
            visible: (Plasmoid.pluginName !== "org.kde.plasma.icontasks")
            text: i18n("Group only when the Task Manager is full")
            enabled: groupingStrategy.currentIndex > 0 && groupPopups.checked
            Accessible.onPressAction: toggle()
        }

        Item {
            Kirigami.FormData.isSection: true
            visible: (Plasmoid.pluginName !== "org.kde.plasma.icontasks")
        }

        QQC2.ComboBox {
            id: sortingStrategy
            Kirigami.FormData.label: i18n("Sort:")
            Layout.fillWidth: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 14
            model: [i18n("Do not sort"), i18n("Manually"), i18n("Alphabetically"), i18n("By desktop"), i18n("By activity")]
        }

        QQC2.CheckBox {
            id: separateLaunchers
            visible: (Plasmoid.pluginName !== "org.kde.plasma.icontasks")
            text: i18n("Keep launchers separate")
            enabled: sortingStrategy.currentIndex === 1
        }

        QQC2.CheckBox {
            id: hideLauncherOnStart
            visible: (Plasmoid.pluginName !== "org.kde.plasma.icontasks")
            text: i18n("Hide launchers after application startup")
        }

        Item {
            Kirigami.FormData.isSection: true
            visible: (Plasmoid.pluginName !== "org.kde.plasma.icontasks")
        }

        QQC2.CheckBox {
            id: minimizeActive
            Kirigami.FormData.label: i18nc("Part of a sentence: 'Clicking active task minimizes the task'", "Clicking active task:")
            text: i18nc("Part of a sentence: 'Clicking active task minimizes the task'", "Minimizes the task")
        }

        QQC2.ComboBox {
            id: middleClickAction
            Kirigami.FormData.label: i18n("Middle-clicking any task:")
            Layout.fillWidth: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 14
            model: [
                i18nc("Part of a sentence: 'Middle-clicking any task does nothing'", "Does nothing"),
                i18nc("Part of a sentence: 'Middle-clicking any task closes window or group'", "Closes window or group"),
                i18nc("Part of a sentence: 'Middle-clicking any task opens a new window'", "Opens a new window"),
                i18nc("Part of a sentence: 'Middle-clicking any task minimizes/restores window or group'", "Minimizes/Restores window or group"),
                i18nc("Part of a sentence: 'Middle-clicking any task toggles grouping'", "Toggles grouping"),
                i18nc("Part of a sentence: 'Middle-clicking any task brings it to the current virtual desktop'", "Brings it to the current virtual desktop")
            ]
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: wheelEnabled
            Kirigami.FormData.label: i18nc("Part of a sentence: 'Mouse wheel cycles through tasks'", "Mouse wheel:")
            text: i18nc("Part of a sentence: 'Mouse wheel cycles through tasks'", "Cycles through tasks")
        }

        RowLayout {
            // HACK: Workaround for Kirigami bug 434625
            // due to which a simple Layout.leftMargin on QQC2.CheckBox doesn't work
            Item { implicitWidth: Kirigami.Units.gridUnit }
            QQC2.CheckBox {
                id: wheelSkipMinimized
                text: i18n("Skip minimized tasks")
                enabled: wheelEnabled.checked
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: showOnlyCurrentScreen
            Kirigami.FormData.label: i18n("Show only tasks:")
            text: i18n("From current screen")
        }

        QQC2.CheckBox {
            id: showOnlyCurrentDesktop
            text: i18n("From current desktop")
        }

        QQC2.CheckBox {
            id: showOnlyCurrentActivity
            text: i18n("From current activity")
        }

        QQC2.CheckBox {
            id: showOnlyMinimized
            text: i18n("That are minimized")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: unhideOnAttention
            Kirigami.FormData.label: i18n("When panel is hidden:")
            text: i18n("Unhide when a window wants attention")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.ButtonGroup {
            id: reverseModeRadioButtonGroup
        }

        QQC2.RadioButton {
            Kirigami.FormData.label: i18n("New tasks appear:")
            checked: !reverseMode.checked
            text: {
                if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    return i18n("On the bottom")
                }
                // horizontal
                if (Qt.application.layoutDirection === Qt.LeftToRight) {
                    return i18n("To the right");
                } else {
                    return i18n("To the left")
                }
            }
            QQC2.ButtonGroup.group: reverseModeRadioButtonGroup
            visible: reverseMode.visible
        }

        QQC2.RadioButton {
            id: reverseMode
            checked: Plasmoid.configuration.reverseMode === true
            text: {
                if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    return i18n("On the Top")
                }
                // horizontal
                if (Qt.application.layoutDirection === Qt.LeftToRight) {
                    return i18n("To the left");
                } else {
                    return i18n("To the right");
                }
            }
            QQC2.ButtonGroup.group: reverseModeRadioButtonGroup
        }

    }
}
