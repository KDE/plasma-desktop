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
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    width: childrenRect.width
    height: childrenRect.height

    property bool vertical: (plasmoid.formFactor == PlasmaCore.Types.Vertical)

    property alias cfg_forceStripes: forceStripes.checked
    property alias cfg_showToolTips: showToolTips.checked
    property alias cfg_wheelEnabled: wheelEnabled.checked
    property alias cfg_highlightWindows: highlightWindows.checked
    property alias cfg_smartLaunchersEnabled: smartLaunchers.checked
    property alias cfg_maxStripes: maxStripes.value
    property alias cfg_groupingStrategy: groupingStrategy.currentIndex
    property alias cfg_middleClickAction: middleClickAction.currentIndex
    property alias cfg_groupPopups: groupPopups.checked
    property alias cfg_onlyGroupWhenFull: onlyGroupWhenFull.checked
    property alias cfg_sortingStrategy: sortingStrategy.currentIndex
    property alias cfg_separateLaunchers: separateLaunchers.checked
    property alias cfg_showOnlyCurrentScreen: showOnlyCurrentScreen.checked
    property alias cfg_showOnlyCurrentDesktop: showOnlyCurrentDesktop.checked
    property alias cfg_showOnlyCurrentActivity: showOnlyCurrentActivity.checked
    property alias cfg_showOnlyMinimized: showOnlyMinimized.checked

    ColumnLayout {
        GroupBox {
            Layout.fillWidth: true

            title: i18n("Arrangement")
            flat: true

            GridLayout {
                columns: 2
                Layout.fillWidth: true

                Label {
                    text: vertical ? i18n("Maximum columns:") : i18n("Maximum rows:")
                }

                SpinBox {
                    id: maxStripes
                    minimumValue: 1
                }

                CheckBox {
                    id: forceStripes
                    Layout.column: 1
                    Layout.row: 1
                    text: vertical ? i18n("Always arrange tasks in rows of as many columns") : i18n("Always arrange tasks in columns of as many rows")
                    enabled: maxStripes.value > 1
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true

            title: i18n("Behavior")
            flat: true

            ColumnLayout {
                Layout.fillWidth: true

                CheckBox {
                    id: showToolTips
                    text: i18n("Show tooltips")
                }

                CheckBox {
                    id: wheelEnabled
                    text: i18n("Cycle through tasks with mouse wheel")
                }

                CheckBox {
                    id: highlightWindows
                    text: i18n("Highlight windows")
                }

                CheckBox {
                    id: smartLaunchers
                    Layout.fillWidth: true
                    text: i18n("Show progress and status information in task buttons")
                }

                RowLayout {
                    Label {
                        text: i18n("On middle-click:")
                    }

                    ComboBox {
                        id: middleClickAction
                        Layout.fillWidth: true
                        model: [i18nc("The click action", "None"), i18n("Close Window or Group"), i18n("New Instance"), i18n("Minimize/Restore Window or Group")]
                    }
                }
            }
        }

        GroupBox {
            Layout.fillWidth: true

            title: i18n("Grouping and Sorting")
            flat: true

            visible: (plasmoid.pluginName != "org.kde.plasma.icontasks")

            ColumnLayout {
                GridLayout {
                    columns: 3
                    Label {
                        Layout.fillWidth: true
                        text: i18n("Sorting:")
                        horizontalAlignment: Text.AlignRight
                    }

                    ComboBox {
                        id: sortingStrategy
                        Layout.fillWidth: true
                        model: [i18n("Do Not Sort"), i18n("Manually"), i18n("Alphabetically"), i18n("By Desktop"), i18n("By Activity")]
                    }

                    CheckBox {
                        id: separateLaunchers
                        Layout.column: 1
                        Layout.row: 1
                        Layout.columnSpan: 2
                        text: i18n("Keep launchers separate")
                        enabled: sortingStrategy.currentIndex == 1
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.row: 2
                        Layout.column: 0
                        text: i18n("Grouping:")
                        horizontalAlignment: Text.AlignRight
                    }

                    ComboBox {
                        id: groupingStrategy
                        Layout.row: 2
                        Layout.column: 1
                        Layout.fillWidth: true
                        model: [i18n("Do Not Group"), i18n("By Program Name")]
                    }

                    CheckBox {
                        id: groupPopups
                        Layout.column: 1
                        Layout.row: 3
                        Layout.columnSpan: 2
                        text: i18n("Open groups in popups")
                        enabled: groupingStrategy.currentIndex > 0
                    }

                    Item {
                        width: childrenRect.width
                        height: childrenRect.height

                        Layout.column: 1
                        Layout.row: 4
                        Layout.columnSpan: 2

                        CheckBox {
                            id: onlyGroupWhenFull
                            anchors.left: parent.left
                            anchors.leftMargin: units.gridUnit
                            text: i18n("Only group when the task manager is full")
                            enabled: groupingStrategy.currentIndex > 0 && groupPopups.checked
                        }
                    }
                }

            }
        }

        GroupBox {
            Layout.fillWidth: true

            title: i18n("Filters")
            flat: true

            ColumnLayout {
                Layout.fillWidth: true

                CheckBox {
                    id: showOnlyCurrentScreen
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

                    visible: (plasmoid.pluginName != "org.kde.plasma.icontasks")

                    text: i18n("Show only tasks that are minimized")
                }
            }
        }
    }
}
