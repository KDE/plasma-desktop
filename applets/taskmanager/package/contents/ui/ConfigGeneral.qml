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

Item {
    width: childrenRect.width
    height: childrenRect.height

    property alias cfg_forceStripes: forceStripes.checked
    property alias cfg_showToolTips: showToolTips.checked
    property alias cfg_wheelEnabled: wheelEnabled.checked
    property alias cfg_highlightWindows: highlightWindows.checked
    property alias cfg_maxStripes: maxStripes.value
    property alias cfg_groupingStrategy: groupingStrategy.currentIndex
    property alias cfg_middleClickAction: middleClickAction.currentIndex
    property alias cfg_onlyGroupWhenFull: onlyGroupWhenFull.checked
    property alias cfg_sortingStrategy: sortingStrategy.currentIndex
    property alias cfg_showOnlyCurrentScreen: showOnlyCurrentScreen.checked
    property alias cfg_showOnlyCurrentDesktop: showOnlyCurrentDesktop.checked
    property alias cfg_showOnlyCurrentActivity: showOnlyCurrentActivity.checked
    property alias cfg_showOnlyMinimized: showOnlyMinimized.checked

    ColumnLayout {
        GroupBox {
            Layout.fillWidth: true

            title: i18n("Appearance")
            flat: true

            ColumnLayout {
                Layout.fillWidth: true

                CheckBox {
                    id: forceStripes
                    text: i18n("Force row settings")
                }

                CheckBox {
                    id: showToolTips
                    text: i18n("Show tooltips")
                }

                RowLayout {
                    Layout.fillHeight: false

                    Label {
                        text: i18n("Maximum rows:")
                    }

                    SpinBox {
                        id: maxStripes
                        minimumValue: 1
                    }
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
                    id: wheelEnabled
                    text: i18n("Cycle through tasks with mouse wheel")
                }

                CheckBox {
                    id: highlightWindows
                    text: i18n("Highlight windows")
                }

                RowLayout {
                    Label {
                        text: i18n("On middle-click:")
                    }

                    ComboBox {
                        id: middleClickAction
                        Layout.fillWidth: true
                        model: [i18nc("The click action", "None"), i18n("Close Window or Group"), i18n("New Instance")]
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
                RowLayout {
                    Label {
                        text: i18n("Sorting:")
                    }

                    ComboBox {
                        id: sortingStrategy

                        Layout.fillWidth: true

                        model: [i18n("Do Not Sort"), i18n("Manually"), i18n("Alphabetically"), i18n("By Desktop"), i18n("By Activity")]
                    }
                }

                RowLayout {
                    Label {
                        text: i18n("Grouping:")
                    }

                    ComboBox {
                        id: groupingStrategy

                        Layout.fillWidth: true

                        model: [i18n("Do Not Group"), i18n("By Program Name")]
                    }
                }

                CheckBox {
                    id: onlyGroupWhenFull
                    text: i18n("Only when the task manager is full")
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
