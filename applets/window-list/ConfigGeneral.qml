/*
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *  SPDX-FileCopyrightText: 2025 Shubham Arora <contact@shubhamarora.dev>
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
    property int cfg_widthStrategy: Plasmoid.configuration.widthStrategy
    property alias cfg_width: widthSpinBox.value

    Kirigami.FormLayout {
        anchors.right: parent.right
        anchors.left: parent.left

        QQC2.ComboBox {
            id: displayModeComboBox

            Kirigami.FormData.label: i18nc("@label:listbox", "For active application, show:")

            enabled: Plasmoid.formFactor === PlasmaCore.Types.Horizontal

            model: [
                i18nc("@item:inlistbox what to show for active application in horizontal panel", "Icon and name"),
                i18nc("@item:inlistbox what to show for active application in horizontal panel", "Icon"),
                i18nc("@item:inlistbox what to show for active application in horizontal panel", "Name")
            ]

            property var mode: 0

            onModeChanged: {
                if (mode === 0) {
                    root.cfg_showIcon = true
                    root.cfg_showText = true
                } else if (mode === 1) {
                    root.cfg_showIcon = true
                    root.cfg_showText = false
                } else if (mode === 2) {
                    root.cfg_showIcon = false
                    root.cfg_showText = true
                }
            }

            onCurrentIndexChanged: mode = currentIndex

            Component.onCompleted: {
                if (root.cfg_showIcon && root.cfg_showText) {
                    mode = 0
                } else if (root.cfg_showIcon && !root.cfg_showText) {
                    mode = 1
                } else if (!root.cfg_showIcon && root.cfg_showText) {
                    mode = 2
                } else {
                    // Invalid state, fallback to icon only
                    mode = 1
                    root.cfg_showIcon = true
                    root.cfg_showText = false
                }
                currentIndex = mode
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            // Maximum length to make it wrap earlier, because long unwrapped text is ugly
            // and harder to read. Keet same as combo box above otherwise there is a 
            // jump in width on text change.
            Layout.maximumWidth: Kirigami.Units.gridUnit * 14

            visible: !displayModeComboBox.enabled
        
            text: Plasmoid.formFactor === PlasmaCore.Types.Vertical ?
                // On a vertical panel
                i18nc("@info:usagetip display mode for active application", "Only icons can be shown when the Panel is vertical.") :
                // On the desktop
                i18nc("@info:usagetip display mode for active application", "Not applicable when the widget is on the Desktop.")
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            font: Kirigami.Theme.smallFont
        }

        Item {
            Kirigami.FormData.isSection: true
        }


        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Kirigami.FormData.label: i18nc("@label:checkbox", "Open on hover:")

            QQC2.CheckBox {
                id: openOnHoverCheckbox

                text: i18nc("@option:check open-on-hover is enabled, with the following delay:", "Enabled, with delay:")
            }

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
                    return Number.fromLocaleString(locale, text.replace(i18ncp("short for millisecond(s)", "ms", "ms"), ""))
                }

                Accessible.name: i18nc("@label:spinbox accessible %1 is milliseconds", "Hover open delay %1 ms", value)
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.CheckBox {
            id: showOnlyCurrentDesktop
            Kirigami.FormData.label: i18nc("@label for checkbox group, completes sentence like: … from current screen", "Show only windows:")
            text: i18nc("@option:check completes sentence: show only windows", "From the current desktop")
        }

        QQC2.CheckBox {
            id: showOnlyCurrentActivity
            text: i18nc("@option:check completes sentence: show only windows", "From the current activity")
        }

        QQC2.CheckBox {
            id: showOnlyCurrentScreen
            text: i18nc("@option:check completes sentence: show only windows", "From the current screen")
        }

        QQC2.CheckBox {
            id: showOnlyMinimized
            text: i18nc("@option:check completes sentence: show only windows", "That are minimized")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.ComboBox {
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
            onActivated: root.cfg_sortingStrategy = currentValue
            Component.onCompleted: currentIndex = indexOfValue(root.cfg_sortingStrategy)
        }

        Component {
            id: listItemDelegate

            QQC2.ItemDelegate {
                id: delegate

                required property string title
                required property string description

                width: parent?.width ?? 0

                text: delegate.title

                contentItem: Kirigami.TitleSubtitle {
                    title: delegate.title
                    subtitle: delegate.description
                    font: delegate.font
                    selected: delegate.highlighted || delegate.down
                    wrapMode: Text.Wrap
                }
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.ComboBox {
            id: widthStrategyComboBox
            Kirigami.FormData.label: i18nc("@label:listbox", "Width mode:")
            Layout.fillWidth: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 14
            textRole: "title"
            valueRole: "value"
            delegate: listItemDelegate

            enabled: Plasmoid.formFactor === PlasmaCore.Types.Horizontal && root.cfg_showText === true
            model: [
                {
                    title: i18nc("@item:inlistbox width strategy for window list widget", "Automatic"),
                    description: i18nc("@item:inlistbox width strategy for window list widget", "Widget width is based on content, expanding and contracting as needed."),
                    value: Globals.WidthStrategy.Automatic
                },
                {
                    title: i18nc("@item:inlistbox width strategy for window list widget", "Fixed"),
                    description: i18nc("@item:inlistbox width strategy for window list widget", "Widget has a fixed width, and text is elided if it does not fit."),
                    value: Globals.WidthStrategy.Fixed
                },
                {
                    title: i18nc("@item:inlistbox width strategy for window list widget", "Fixed Maximum"), 
                    description: i18nc("@item:inlistbox width strategy for window list widget", "Widget has a fixed maximum width with minimum width based on content, and text is elided if it does not fit."),
                    value: Globals.WidthStrategy.FixedMaximum
                }
            ]

            onActivated: root.cfg_widthStrategy = currentValue

            Component.onCompleted: currentIndex = indexOfValue(root.cfg_widthStrategy)
        }

        QQC2.Label {
            Layout.fillWidth: true
            // Maximum length to make it wrap earlier, because long unwrapped text is ugly
            // and harder to read. Keet same as combo box above otherwise there is a 
            // jump in width on text change.
            Layout.maximumWidth: Kirigami.Units.gridUnit * 14

            visible: !widthStrategyComboBox.enabled
        
            text: {
                if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    // On a vertical panel
                    return i18n("Not applicable when the Panel is vertical.")
                } else if (root.cfg_showIcon === true && root.cfg_showText === false) {
                    // Only icons are shown in horizontal panel
                    return i18n("Not applicable when only icons are shown in the horizontal Panel.")
                } else  {
                    // On the desktop
                    return i18n("Not applicable when the widget is on the Desktop.")    
                }
            }
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            font: Kirigami.Theme.smallFont
        }

        QQC2.SpinBox {
            id: widthSpinBox
            Kirigami.FormData.label: i18nc("@label:spinbox", "Fixed width:")

            enabled: widthStrategyComboBox.enabled && (widthStrategyComboBox.currentValue === Globals.WidthStrategy.Fixed || widthStrategyComboBox.currentValue === Globals.WidthStrategy.FixedMaximum)

            from: 5
            to: 50
            stepSize: 1
            
            Accessible.name: i18nc("@label:spinbox accessible", "Widget width")
        }

        QQC2.Label {
            Layout.fillWidth: true
            // Maximum length to make it wrap earlier, because long unwrapped text is ugly
            // and harder to read. Keet same as combo box above otherwise there is a 
            // jump in width on text change.
            Layout.maximumWidth: Kirigami.Units.gridUnit * 14

            visible: !(widthStrategyComboBox.enabled) || widthStrategyComboBox.currentValue === Globals.WidthStrategy.Automatic
        
            text: {
                if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    // On a vertical panel
                    return i18n("Not applicable when the Panel is vertical.")
                } else if (root.cfg_showIcon === true && root.cfg_showText === false) {
                    // Only icons are shown in horizontal panel
                    return i18n("Not applicable when only icons are shown in the horizontal Panel.")
                } else  if (Plasmoid.formFactor === PlasmaCore.Types.Horizontal && widthStrategyComboBox.currentValue === Globals.WidthStrategy.Automatic) {
                    // Automatic width strategy in horizontal panel
                    return i18n("Not applicable when the width mode is set to Automatic.")
                } else {
                    // On the desktop
                    return i18n("Not applicable when the widget is on the Desktop.")    
                }
            }
            
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            font: Kirigami.Theme.smallFont
        }

    }
}
