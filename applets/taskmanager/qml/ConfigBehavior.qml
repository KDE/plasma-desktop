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
import org.kde.taskmanager as TaskManager

KCMUtils.SimpleKCM {
    id: root

    property alias cfg_groupingStrategy: groupingStrategy.currentIndex
    property alias cfg_groupedTaskVisualization: groupedTaskVisualization.currentIndex
    property alias cfg_groupPopups: groupPopups.checked
    property alias cfg_onlyGroupWhenFull: onlyGroupWhenFull.checked
    property int cfg_sortingStrategy
    property alias cfg_separateLaunchers: separateLaunchers.checked
    property alias cfg_hideLauncherOnStart: hideLauncherOnStart.checked
    property alias cfg_middleClickAction: middleClickAction.currentIndex
    property alias cfg_wheelEnabled: wheelEnabled.currentIndex
    property alias cfg_wheelSkipMinimized: wheelSkipMinimized.checked
    property alias cfg_showOnlyCurrentScreen: showOnlyCurrentScreen.checked
    property alias cfg_showOnlyCurrentDesktop: showOnlyCurrentDesktop.checked
    property alias cfg_showOnlyCurrentActivity: showOnlyCurrentActivity.checked
    property alias cfg_showOnlyMinimized: showOnlyMinimized.checked
    property alias cfg_minimizeActiveTaskOnClick: minimizeActive.checked
    property alias cfg_unhideOnAttention: unhideOnAttention.checked
    property alias cfg_reverseMode: reverseMode.checked

    headerPaddingEnabled: false
    header: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.InlineMessage {
            id: annoyingAppWorkaroundMessage

            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header

            text: i18nc("@info", "If you're using this setting to work around an application that demands attention too often, first look for a setting in the app to disable that behavior. If you don't find one, consider reporting this as a bug to the app's developer.")
        }

            // "You asked for Window View but Window View is not available" message
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: groupedTaskVisualization.currentIndex === 2 && !effectWatcher.registered
            type: Kirigami.MessageType.Warning
            text: i18nc("@info displayed as InlineMessage", "The compositor does not support displaying windows side by side, so a textual list will be displayed instead.")
        }
    }

    DBus.DBusServiceWatcher {
        id: effectWatcher
        busType: DBus.BusType.Session
        watchedService: "org.kde.KWin.Effect.WindowView1"
    }

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label:listbox how to group tasks", "Group:")
                contentItem: QQC2.ComboBox {
                    id: groupingStrategy
                    Layout.fillWidth: true
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 14
                    model: [
                        i18nc("@item:inlistbox how to group tasks", "Do not group"),
                        i18nc("@item:inlistbox how to group tasks", "By program name")
                    ]
                }
            }
            Kirigami.FormEntry {
                visible: (Plasmoid.pluginName !== "org.kde.plasma.icontasks")
                contentItem: QQC2.CheckBox {
                    id: groupPopups
                    text: i18nc("@option:check grouped task", "Combine into single button")
                    enabled: groupingStrategy.currentIndex > 0
                }
            }
            Kirigami.FormEntry {
                visible: (Plasmoid.pluginName !== "org.kde.plasma.icontasks")
                contentItem: QQC2.CheckBox {
                    id: onlyGroupWhenFull
                    text: i18nc("@option:check grouped task","Group only when the Task Manager is full")
                    enabled: groupingStrategy.currentIndex > 0 && groupPopups.checked
                    Accessible.onPressAction: toggle()
                }
            }

            Kirigami.FormEntry {
                title: i18nc("@label:listbox completes sentence like: … cycles through tasks", "Clicking grouped task:")
                contentItem: QQC2.ComboBox {
                    id: groupedTaskVisualization
                    Layout.fillWidth: true
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 14

                    enabled: groupingStrategy.currentIndex !== 0

                    model: [
                        i18nc("@item:inlistbox Completes the sentence 'Clicking grouped task cycles through tasks' ", "Cycles through tasks"),
                        i18nc("@item:inlistbox Completes the sentence 'Clicking grouped task shows small window previews' ", "Shows small window previews"),
                        i18nc("@item:inlistbox Completes the sentence 'Clicking grouped task shows large window previews' ", "Shows large window previews"),
                        i18nc("@item:inlistbox Completes the sentence 'Clicking grouped task shows textual list' ", "Shows textual list"),
                    ]

                    Accessible.name: currentText
                    Accessible.onPressAction: currentIndex = currentIndex === count - 1 ? 0 : (currentIndex + 1)
                }
            }
        }
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label:listbox sort tasks in grouped task", "Sort:")
                contentItem: QQC2.ComboBox {
                    id: sortingStrategy
                    Layout.fillWidth: true
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 14
                    textRole: "text"
                    valueRole: "value"
                    model: [
                        {
                            "text": i18nc("@item:inlistbox sort tasks in grouped task", "Do not sort"),
                            "value": TaskManager.TasksModel.SortDisabled,
                        },
                        {
                            "text": i18nc("@item:inlistbox sort tasks in grouped task", "Manually"),
                            "value": TaskManager.TasksModel.SortManual,
                        },
                        {
                            "text": i18nc("@item:inlistbox sort tasks in grouped task", "Alphabetically"),
                            "value": TaskManager.TasksModel.SortAlpha,
                        },
                        {
                            "text": i18nc("@item:inlistbox sort tasks in grouped task", "By desktop"),
                            "value": TaskManager.TasksModel.SortVirtualDesktop,
                        },
                        {
                            "text": i18nc("@item:inlistbox sort tasks in grouped task", "By activity"),
                            "value": TaskManager.TasksModel.SortActivity,
                        },
                        {
                            "text": i18nc("@item:inlistbox sort tasks in grouped task", "By horizontal window position"),
                            "value": TaskManager.TasksModel.SortWindowPositionHorizontal,
                        },
                    ]
                    onActivated: root.cfg_sortingStrategy = currentValue
                    Component.onCompleted: currentIndex = indexOfValue(root.cfg_sortingStrategy)
                }
            }

            Kirigami.FormEntry {
                visible: (Plasmoid.pluginName !== "org.kde.plasma.icontasks")
                contentItem: QQC2.CheckBox {
                    id: separateLaunchers
                    text: i18nc("@option:check configure task sorting", "Keep launchers separate")
                    enabled: sortingStrategy.currentValue === TaskManager.TasksModel.SortManual
                }
            }

            Kirigami.FormEntry {
                visible: (Plasmoid.pluginName !== "org.kde.plasma.icontasks")
                contentItem: QQC2.CheckBox {
                    id: hideLauncherOnStart
                    text: i18nc("@option:check for icons-and-text task manager", "Hide launchers after application startup")
                }
            }

        }
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label for checkbox Part of a sentence: 'Clicking active task minimizes the task'", "Clicking active task:")
                contentItem: QQC2.CheckBox {
                    id: minimizeActive
                    text: i18nc("@option:check Part of a sentence: 'Clicking active task minimizes the task'", "Minimizes the task")
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@label:listbox completes sentence like: … does nothing", "Middle-clicking any task:")
                contentItem: QQC2.ComboBox {
                    id: middleClickAction
                    Layout.fillWidth: true
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 14
                    model: [
                        i18nc("@item:inlistbox Part of a sentence: 'Middle-clicking any task does nothing'", "Does nothing"),
                        i18nc("@item:inlistbox Part of a sentence: 'Middle-clicking any task closes window or group'", "Closes window or group"),
                        i18nc("@item:inlistbox Part of a sentence: 'Middle-clicking any task opens a new window'", "Opens a new window"),
                        i18nc("@item:inlistbox Part of a sentence: 'Middle-clicking any task minimizes/restores window or group'", "Minimizes/Restores window or group"),
                        i18nc("@item:inlistbox Part of a sentence: 'Middle-clicking any task toggles grouping'", "Toggles grouping"),
                        i18nc("@item:inlistbox Part of a sentence: 'Middle-clicking any task brings it to the current virtual desktop'", "Brings it to the current virtual desktop")
                    ]
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@label:listbox Part of a sentence: 'Scrolling behavior does nothing/cycles through tasks/cycles through the selected task's windows/adjusts the hovered task’s volume''", "Scrolling behavior:")
                contentItem: QQC2.ComboBox {
                    id: wheelEnabled
                    Layout.fillWidth: true
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 14
                    model: [
                        i18nc("@item:inlistbox Part of a sentence: 'Scrolling behavior does nothing'", "Does nothing"),
                        i18nc("@item:inlistbox Part of a sentence: 'Scrolling behavior cycles through all tasks'", "Cycles through all tasks"),
                        i18nc("@item:inlistbox Part of a sentence: 'Scrolling behavior cycles through the hovered task's windows'", "Cycles through the hovered task’s windows"),
                        i18nc("@item:inlistbox Part of a sentence: 'Scrolling behavior adjusts the hovered task’s volume'", "Adjusts the hovered task’s volume"),
                    ]
                }
            }
            Kirigami.FormEntry {
                enabled: wheelEnabled.currentIndex !== 0 && wheelEnabled.currentIndex !== 3 // Nothing or volume
                contentItem: QQC2.CheckBox {
                    id: wheelSkipMinimized
                    leftPadding: mirrored ? 0 : (wheelEnabled.indicator.width + wheelEnabled.spacing)
                    rightPadding: mirrored ? (wheelEnabled.indicator.width + wheelEnabled.spacing) : 0
                    text: i18nc("@option:check mouse wheel task cycling", "Skip minimized tasks")
                }
            }
        }

        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label for checkbox group, completes sentence like: … from current screen", "Show only tasks:")
                contentItem: QQC2.CheckBox {
                    id: showOnlyCurrentDesktop
                    text: i18nc("@option:check completes sentence: show only tasks", "From the current desktop")
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: showOnlyCurrentActivity
                    text: i18nc("@option:check completes sentence: show only tasks", "From the current activity")
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: showOnlyCurrentScreen
                    text: i18nc("@option:check completes sentence: show only tasks", "From the current screen")
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: showOnlyMinimized
                    text: i18nc("@option:check completes sentence: show only tasks", "That are minimized")
                }
            }
        }
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label for checkbox, completes sentence: … unhide if window wants attention", "When panel is hidden:")
                contentItem: QQC2.CheckBox {
                    id: unhideOnAttention
                    text: i18nc("@option:check completes sentence: When panel is hidden", "Unhide when a window wants attention")
                    onToggled: {
                        annoyingAppWorkaroundMessage.visible = !unhideOnAttention.checked;
                    }
                }
            }
        }
        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: reverseModeRadioButtonGroup }
            Kirigami.FormEntry {
                title: i18nc("@label for radiobutton group completes sentence like: … on the bottom", "New tasks appear:")
                contentItem: QQC2.RadioButton {
                    checked: !reverseMode.checked
                    text: {
                        if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                            return i18nc("@option:check completes sentence: New tasks appear", "On the bottom")
                        }
                        // horizontal
                        if (Application.layoutDirection === Qt.LeftToRight) {
                            return i18nc("@option:check completes sentence: New tasks appear", "To the right");
                        } else {
                            return i18nc("@option:check completes sentence: New tasks appear", "To the left")
                        }
                    }
                    QQC2.ButtonGroup.group: reverseModeRadioButtonGroup
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    id: reverseMode
                    checked: Plasmoid.configuration.reverseMode === true
                    text: {
                        if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                            return i18nc("@option:check completes sentence: New tasks appear", "On the top")
                        }
                        // horizontal
                        if (Application.layoutDirection === Qt.LeftToRight) {
                            return i18nc("@option:check completes sentence: New tasks appear", "To the left");
                        } else {
                            return i18nc("@option:check completes sentence: New tasks appear", "To the right");
                        }
                    }
                    QQC2.ButtonGroup.group: reverseModeRadioButtonGroup
                }
            }
        }
    }
}
