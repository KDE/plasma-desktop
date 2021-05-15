/***************************************************************************
 *   Copyright (C) 2013 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.4 as Kirigami

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.private.taskmanager 0.1 as TaskManagerApplet

Item {
    width: childrenRect.width
    height: childrenRect.height

    property alias cfg_groupingStrategy: groupingStrategy.currentIndex
    property alias cfg_groupedTaskVisualization: groupedTaskVisualization.currentIndex
    property alias cfg_groupPopups: groupPopups.checked
    property alias cfg_onlyGroupWhenFull: onlyGroupWhenFull.checked
    property alias cfg_sortingStrategy: sortingStrategy.currentIndex
    property alias cfg_separateLaunchers: separateLaunchers.checked
    property alias cfg_middleClickAction: middleClickAction.currentIndex
    property alias cfg_wheelEnabled: wheelEnabled.checked
    property alias cfg_showOnlyCurrentScreen: showOnlyCurrentScreen.checked
    property alias cfg_showOnlyCurrentDesktop: showOnlyCurrentDesktop.checked
    property alias cfg_showOnlyCurrentActivity: showOnlyCurrentActivity.checked
    property alias cfg_showOnlyMinimized: showOnlyMinimized.checked
    property alias cfg_minimizeActiveTaskOnClick: minimizeActive.checked
    property alias cfg_unhideOnAttention: unhideOnAttention.checked

    TaskManagerApplet.Backend {
        id: backend
    }

    Kirigami.FormLayout {
        anchors.left: parent.left
        anchors.right: parent.right

        ComboBox {
            id: groupingStrategy
            Kirigami.FormData.label: i18n("Group:")
            Layout.fillWidth: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 14
            model: [i18n("Do not group"), i18n("By program name")]
        }

        ComboBox {
            id: groupedTaskVisualization
            Kirigami.FormData.label: i18n("Clicking grouped task:")
            Layout.fillWidth: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 14

            enabled: groupingStrategy.currentIndex !== 0

            model: [
                i18nc("Completes the sentence 'Clicking grouped task cycles through tasks' ", "Cycles through tasks"),
                i18nc("Completes the sentence 'Clicking grouped task shows tooltip window thumbnails' ", "Shows tooltip window thumbnails"),
                i18nc("Completes the sentence 'Clicking grouped task shows 'Present Windows' effect' ", "Shows 'Present Windows' effect"),
                i18nc("Completes the sentence 'Clicking grouped task shows textual list' ", "Shows textual list"),
            ]
        }
        // "You asked for Tooltips but Tooltips are disabled" message
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: groupedTaskVisualization.currentIndex === 1 && !plasmoid.configuration.showToolTips && backend.canPresentWindows
            type: Kirigami.MessageType.Warning
            text: i18n("Tooltips are disabled, so the 'Present Windows' effect will be displayed instead.")
        }
        // "You asked for Tooltips but Tooltips are disabled and Present Windows is not available" message
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: groupedTaskVisualization.currentIndex === 1 && !plasmoid.configuration.showToolTips && !backend.canPresentWindows
            type: Kirigami.MessageType.Warning
            text: i18n("Tooltips are disabled, and the 'Present Windows' effect is not enabled or otherwise available right now, so a textual list will be displayed instead")
        }
        // "You asked for Present Windows but Present Windows is not available" message
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: groupedTaskVisualization.currentIndex === 2 && !backend.canPresentWindows
            type: Kirigami.MessageType.Warning
            text: i18n("The 'Present Windows' effect is not enabled or otherwise available right now, so a textual list will be displayed instead.")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        CheckBox {
            id: groupPopups
            visible: (plasmoid.pluginName !== "org.kde.plasma.icontasks")
            text: i18n("Combine into single button")
            enabled: groupingStrategy.currentIndex > 0
        }

        CheckBox {
            id: onlyGroupWhenFull
            visible: (plasmoid.pluginName !== "org.kde.plasma.icontasks")
            text: i18n("Group only when the Task Manager is full")
            enabled: groupingStrategy.currentIndex > 0 && groupPopups.checked
        }

        Item {
            Kirigami.FormData.isSection: true
            visible: (plasmoid.pluginName !== "org.kde.plasma.icontasks")
        }

        ComboBox {
            id: sortingStrategy
            Kirigami.FormData.label: i18n("Sort:")
            Layout.fillWidth: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 14
            model: [i18n("Do not sort"), i18n("Manually"), i18n("Alphabetically"), i18n("By desktop"), i18n("By activity")]
        }

        CheckBox {
            id: separateLaunchers
            visible: (plasmoid.pluginName !== "org.kde.plasma.icontasks")
            text: i18n("Keep launchers separate")
            enabled: sortingStrategy.currentIndex == 1
        }

        Item {
            Kirigami.FormData.isSection: true
            visible: (plasmoid.pluginName !== "org.kde.plasma.icontasks")
        }

        CheckBox {
            id: minimizeActive
            Kirigami.FormData.label: i18nc("Part of a sentence: 'Clicking active task minimizes the task'", "Clicking active task:")
            text: i18nc("Part of a sentence: 'Clicking active task minimizes the task'", "Minimizes the task")
        }

        ComboBox {
            id: middleClickAction
            Kirigami.FormData.label: i18n("Middle-clicking any task:")
            Layout.fillWidth: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 14
            model: [
                i18nc("Part of a sentence: 'Middle-clicking any task does nothing'", "Does nothing"),
                i18nc("Part of a sentence: 'Middle-clicking any task closes window or group'", "Closes window or group"),
                i18nc("Part of a sentence: 'Middle-clicking any task opens a new instance'", "Opens a new instance"),
                i18nc("Part of a sentence: 'Middle-clicking any task minimizes/restores window or group'", "Minimizes/Restores window or group"),
                i18nc("Part of a sentence: 'Middle-clicking any task toggles grouping'", "Toggles grouping"),
                i18nc("Part of a sentence: 'Middle-clicking any task brings it to the current virtual desktop'", "Brings it to the current virtual desktop")
            ]
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        CheckBox {
            id: wheelEnabled
            Kirigami.FormData.label: i18nc("Part of a sentence: 'Mouse wheel cycles through tasks'", "Mouse wheel:")
            text: i18nc("Part of a sentence: 'Mouse wheel cycles through tasks'", "Cycles through tasks")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        CheckBox {
            id: showOnlyCurrentScreen
            Kirigami.FormData.label: i18n("Show only tasks:")
            text: i18n("From current screen")
        }

        CheckBox {
            id: showOnlyCurrentDesktop
            text: i18n("From current desktop")
        }

        CheckBox {
            id: showOnlyCurrentActivity
            text: i18n("From current activity")
        }

        CheckBox {
            id: showOnlyMinimized
            text: i18n("That are minimized")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        CheckBox {
            id: unhideOnAttention
            Kirigami.FormData.label: i18n("When panel is hidden:")
            text: i18n("Unhide when a window wants attention")
        }
    }
}
