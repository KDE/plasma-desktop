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
import QtQuick.Controls 1.4 as QQC1
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.4 as Kirigami

import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    width: childrenRect.width
    height: childrenRect.height

    property alias cfg_groupingStrategy: groupingStrategy.currentIndex
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

    Kirigami.FormLayout {
        anchors.left: parent.left
        anchors.right: parent.right

        // TODO: port to QQC2 version once we've fixed https://bugs.kde.org/show_bug.cgi?id=403153
        QQC1.ComboBox {
            id: groupingStrategy
            visible: (plasmoid.pluginName !== "org.kde.plasma.icontasks")
            Kirigami.FormData.label: i18n("Group:")
            Layout.fillWidth: true
            model: [i18n("Do not group"), i18n("By program name")]
        }

        CheckBox {
            id: groupPopups
            visible: (plasmoid.pluginName !== "org.kde.plasma.icontasks")
            text: i18n("Open groups in popups")
            enabled: groupingStrategy.currentIndex > 0
        }

        RowLayout {
            visible: (plasmoid.pluginName !== "org.kde.plasma.icontasks")

            // Indent the option as it depends on the previous one
            Item {
                width: units.largeSpacing
            }

            CheckBox {
                id: onlyGroupWhenFull
                text: i18n("Group only when the Task Manager is full")
                enabled: groupingStrategy.currentIndex > 0 && groupPopups.checked
            }
        }

        Item {
            Kirigami.FormData.isSection: true
            visible: (plasmoid.pluginName !== "org.kde.plasma.icontasks")
        }

        // TODO: port to QQC2 version once we've fixed https://bugs.kde.org/show_bug.cgi?id=403153
        QQC1.ComboBox {
            id: sortingStrategy
            visible: (plasmoid.pluginName !== "org.kde.plasma.icontasks")
            Kirigami.FormData.label: i18n("Sort:")
            Layout.fillWidth: true
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

        // TODO: port to QQC2 version once we've fixed https://bugs.kde.org/show_bug.cgi?id=403153
        QQC1.ComboBox {
            id: middleClickAction
            Kirigami.FormData.label: i18n("On middle-click:")
            Layout.fillWidth: true
            model: [
                i18nc("The click action", "None"),
                i18n("Close window or group"),
                i18n("New instance"),
                i18n("Minimize/Restore window or group"),
                i18nc("When clicking it would toggle grouping windows of a specific app", "Group/Ungroup"),
                i18n("Bring to the current virtual desktop")
            ]
        }

        CheckBox {
            id: wheelEnabled
            text: i18n("Cycle through tasks with mouse wheel")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        CheckBox {
            id: showOnlyCurrentScreen
            Kirigami.FormData.label: i18n("Filter:")
            text: i18n("Show only tasks from the current screen")
        }

        CheckBox {
            id: showOnlyCurrentDesktop
            text: i18n("Show only tasks from the current desktop")
        }

        CheckBox {
            id: showOnlyCurrentActivity
            text: i18n("Show only tasks from the current activity")
        }

        CheckBox {
            id: showOnlyMinimized
            visible: (plasmoid.pluginName !== "org.kde.plasma.icontasks")
            text: i18n("Show only tasks that are minimized")
        }
    }
}
