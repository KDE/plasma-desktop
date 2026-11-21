/*
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import org.kde.taskmanager as TaskManager

KCM.SimpleKCM {
    id: root

    property bool cfg_showText: Plasmoid.configuration.showText
    property bool cfg_showIcon: Plasmoid.configuration.showIcon
    property alias cfg_openOnHover: openOnHoverCheckbox.checked
    property alias cfg_hoverOpenDelay: hoverDelaySpinBox.value
    property int cfg_sortingStrategy: Plasmoid.configuration.sortingStrategy
    property alias cfg_showOnlyCurrentScreen: showOnlyCurrentScreen.checked
    property alias cfg_showOnlyCurrentDesktop: showOnlyCurrentDesktop.checked
    property alias cfg_showOnlyCurrentActivity: showOnlyCurrentActivity.checked
    property alias cfg_showOnlyMinimized: showOnlyMinimized.checked

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label:listbox", "For active application, show:")
                contentItem: QQC2.ComboBox {
                    id: displayModeComboBox

                    enabled: Plasmoid.formFactor === PlasmaCore.Types.Horizontal

                    model: [
                        i18nc("@item:inlistbox what to show for active application in horizontal panel", "Icon and name"),
                        i18nc("@item:inlistbox what to show for active application in horizontal panel", "Icon"),
                        i18nc("@item:inlistbox what to show for active application in horizontal panel", "Name")
                    ]

                    onActivated: {
                        root.cfg_showIcon = currentIndex === 0 || currentIndex === 1
                        root.cfg_showText = currentIndex === 1 || currentIndex === 2
                    }

                    currentIndex: {
                        if (root.cfg_showIcon && root.cfg_showText) {
                            return 0
                        } else if (root.cfg_showIcon && !root.cfg_showText) {
                            return 1
                        } else if (!root.cfg_showIcon && root.cfg_showText) {
                            return 2
                        } else {
                            // Invalid state, fallback to icon only
                            return 1
                        }
                    }
                }
                subtitle: if (displayModeComboBox.enabled) {
                    return ""
                } else if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    return i18nc("@info:usagetip display mode for active application",
                                 "Only icons can be shown when the Panel is vertical.")
                } else {
                    return i18nc("@info:usagetip display mode for active application",
                                 "Not applicable when the widget is on the Desktop.")
                }
            }
        }

        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label:checkbox completes … open afer (configurable delay)", "When hovered:")
                contentItem: QQC2.CheckBox {
                    id: openOnHoverCheckbox
                    text: i18nc("@option:check open-on-hover is enabled, with the following delay:", "Open after")
                }
                trailingItems: [
                    QQC2.SpinBox {
                        id: hoverDelaySpinBox

                        from: 0
                        to: 1000
                        stepSize: 50

                        enabled: root.cfg_openOnHover

                        textFromValue: function(value, locale) {
                            return i18ncp("@item:valuesuffix, %1 is hover activation delay in ms", "%1 ms", "%1 ms", value)
                        }

                        validator: IntValidator {
                            bottom: hoverDelaySpinBox.from
                            top: hoverDelaySpinBox.to
                        }

                        valueFromText: (text, locale) => {
                            return Number.fromLocaleString(locale, text.replace(i18ncp("@item:valuesuffix short for millisecond(s)", "ms", "ms"), ""))
                        }

                        Accessible.name: i18nc("@label:spinbox accessible name", "Hover open delay")
                    }
                ]
            }
        }

        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label for checkbox group, completes sentence like: … from current screen", "Show only windows:")
                contentItem: QQC2.CheckBox {
                    id: showOnlyCurrentDesktop
                    text: i18nc("@option:check completes sentence: show only windows", "From the current desktop")
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: showOnlyCurrentActivity
                    text: i18nc("@option:check completes sentence: show only windows", "From the current activity")
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: showOnlyCurrentScreen
                    text: i18nc("@option:check completes sentence: show only windows", "From the current screen")
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: showOnlyMinimized
                    text: i18nc("@option:check completes sentence: show only windows", "That are minimized")
                }
            }
        }

        Kirigami.FormGroup {
            Kirigami.FormEntry {
                contentItem: QQC2.ComboBox {
                    id: sortingStrategy
                    Kirigami.FormData.label: i18nc("@label:listbox sort windows is list", "Sort:")
                    Layout.fillWidth: true
                    Layout.minimumWidth: Kirigami.Units.gridUnit * 14
                    textRole: "text"
                    valueRole: "value"
                    model: [
                        {
                            "text": i18nc("@item:inlistbox sort windows in list", "By time opened"),
                            "value": TaskManager.TasksModel.SortDisabled
                        },
                        {
                            "text": i18nc("@item:inlistbox sort windows in list", "Alphabetically"),
                            "value": TaskManager.TasksModel.SortAlpha
                        },
                        {
                            "text": i18nc("@item:inlistbox sort windows in list", "Alphabetically, grouped by virtual desktop"),
                            "value": TaskManager.TasksModel.SortVirtualDesktop
                        },
                        {
                            "text": i18nc("@item:inlistbox sort windows in list", "Alphabetically, grouped by activity"),
                            "value": TaskManager.TasksModel.SortActivity
                        },
                        {
                            "text": i18nc("@item:inlistbox sort windows in list", "By last activated time"),
                            "value": TaskManager.TasksModel.SortLastActivated
                        },
                        {
                            "text": i18nc("@item:inlistbox sort windows in list", "By horizontal window position"),
                            "value": TaskManager.TasksModel.SortWindowPositionHorizontal
                        }
                    ]
                    Component.onCompleted: currentIndex = indexOfValue(root.cfg_sortingStrategy)
                    onActivated: root.cfg_sortingStrategy = currentValue
                }
            }
        }
    }
}
