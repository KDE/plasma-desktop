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

KCMUtils.SimpleKCM {
    id: root

    readonly property bool plasmaPaAvailable: Qt.createComponent("PulseAudio.qml").status === Component.Ready
    readonly property bool plasmoidVertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical
    readonly property bool iconOnly: Plasmoid.pluginName === "org.kde.plasma.icontasks"

    property alias cfg_showToolTips: showToolTips.checked
    property alias cfg_highlightWindows: highlightWindows.checked
    property bool cfg_indicateAudioStreams
    property bool cfg_interactiveMute
    property bool cfg_tooltipControls
    property alias cfg_fill: fill.checked
    property alias cfg_maxStripes: maxStripes.value
    property alias cfg_forceStripes: forceStripes.checked
    property alias cfg_taskMaxWidth: taskMaxWidth.currentIndex
    property int cfg_iconSpacing: 0

    Component.onCompleted: {
        /* Don't rely on bindings for checking the radiobuttons
           When checking forceStripes, the condition for the checked value for the allow stripes button
           became true and that one got checked instead, stealing the checked state for the just clicked checkbox
        */
        if (maxStripes.value === 1) {
            forbidStripes.checked = true;
        } else if (!Plasmoid.configuration.forceStripes && maxStripes.value > 1) {
            allowStripes.checked = true;
        } else if (Plasmoid.configuration.forceStripes && maxStripes.value > 1) {
            forceStripes.checked = true;
        }
    }
    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@title:group for several checkboxes", "In task tooltips:")
                contentItem: QQC2.CheckBox {
                    id: showToolTips
                    Layout.fillWidth: true
                    text: i18nc("@option:check section General", "Show small window previews")
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: highlightWindows
                    Layout.fillWidth: true
                    text: showToolTips.checked
                        ? i18nc("@option:check completes In task tooltips …", "Hide other windows when hovering over preview")
                        : i18nc("@option:check completes In task tooltips …", "Hide other windows when hovering over task")
                }
            }
            Kirigami.FormEntry {
                enabled: root.plasmaPaAvailable
                contentItem: QQC2.CheckBox {
                    id: tooltipControls
                    Layout.fillWidth: true
                    text: i18nc("@option:check completes In task tooltips …", "Show media and volume controls")
                    checked: root.cfg_tooltipControls && root.plasmaPaAvailable
                    onToggled: root.cfg_tooltipControls = checked
                }
            }
        }
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@title:group for pair of checkboxes", "On tasks playing audio:")
                enabled: root.plasmaPaAvailable
                contentItem: QQC2.CheckBox {
                    id: indicateAudioStreams
                    Layout.fillWidth: true
                    text: i18nc("@option:check On tasks playing audio…", "Show an indicator on task icon")
                    checked: root.cfg_indicateAudioStreams && root.plasmaPaAvailable
                    onToggled: root.cfg_indicateAudioStreams = checked
                }
            }
            Kirigami.FormEntry {
                enabled: indicateAudioStreams.checked && root.plasmaPaAvailable
                contentItem: QQC2.CheckBox {
                    id: interactiveMute
                    Layout.fillWidth: true
                    leftPadding: mirrored ? 0 : (indicateAudioStreams.indicator.width + indicateAudioStreams.spacing)
                    rightPadding: mirrored ? (indicateAudioStreams.indicator.width + indicateAudioStreams.spacing) : 0
                    text: i18nc("@option:check On tasks playing audio…", "Mute task when clicking indicator")
                    checked: root.cfg_interactiveMute && root.plasmaPaAvailable
                    onToggled: root.cfg_interactiveMute = checked
                }
            }
        }
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@option:check prefix, completes … Fill free space", "In panel:")
                contentItem: QQC2.CheckBox {
                    id: fill
                    Layout.fillWidth: true
                    text: i18nc("@option:check completes In panel …", "Fill available free space")
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@label:listbox", "Maximum task width:")
                visible: !root.iconOnly && !root.plasmoidVertical
                contentItem: QQC2.ComboBox {
                    id: taskMaxWidth


                    model: [
                        i18nc("@item:inlistbox how wide a task item should be", "Narrow"),
                        i18nc("@item:inlistbox how wide a task item should be", "Medium"),
                        i18nc("@item:inlistbox how wide a task item should be", "Wide")
                    ]
                }
            }
            Kirigami.FormEntry {
                title: i18nc("@label:listbox", "Spacing between icons:")
                visible: root.iconOnly
                contentItem: QQC2.ComboBox {

                    model: [
                        {
                            "label": i18nc("@item:inlistbox Icon spacing", "Small"),
                            "spacing": 0
                        },
                        {
                            "label": i18nc("@item:inlistbox Icon spacing", "Normal"),
                            "spacing": 1
                        },
                        {
                            "label": i18nc("@item:inlistbox Icon spacing", "Large"),
                            "spacing": 3
                        },
                    ]

                    textRole: "label"

                    currentIndex: switch (root.cfg_iconSpacing) {
                        case 0: return 0; // Small
                        case 1: return 1; // Normal
                        case 3: return 2; // Large
                    }
                    onActivated: index => {
                        root.cfg_iconSpacing = model[currentIndex]["spacing"];
                    }
                }
            }
        }
        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: stripeButtonGroup}
            Kirigami.FormEntry {
                title: root.plasmoidVertical
                    ? i18nc("@label for radio button group, completes sentence: … when panel is low on space etc.", "Use multi-column view:")
                    : i18nc("@label for radio button group, completes sentence: … when panel is low on space etc.", "Use multi-row view:")
                contentItem: QQC2.RadioButton {
                    id: forbidStripes
                    QQC2.ButtonGroup.group: stripeButtonGroup
                    Layout.fillWidth: true
                    onToggled: {
                        if (checked) {
                            maxStripes.value = 1
                        }
                    }
                    text: i18nc("@option:radio Never use multi-column view for Task Manager", "Never")
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    id: allowStripes
                    QQC2.ButtonGroup.group: stripeButtonGroup
                    Layout.fillWidth: true
                    onToggled: {
                        if (checked) {
                            maxStripes.value = Math.max(2, maxStripes.value)
                        }
                    }
                    text: i18nc("@option:radio completes sentence: Use multi-column/row view",
                                "When panel is low on space and thick enough")
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    id: forceStripes
                    QQC2.ButtonGroup.group: stripeButtonGroup
                    Layout.fillWidth: true
                    onToggled: {
                        if (checked) {
                            maxStripes.value = Math.max(2, maxStripes.value)
                        }
                    }
                    text: i18nc("@option:radio completes sentence: Use multi-column/row view",
                                "Always when panel is thick enough")
                }
            }
            Kirigami.FormEntry {
                title: root.plasmoidVertical
                    ? i18nc("@label:spinbox maximum number of columns for tasks", "Maximum columns:")
                    : i18nc("@label:spinbox maximum number of rows for tasks", "Maximum rows:")
                enabled: maxStripes.value > 1
                contentItem: QQC2.SpinBox {
                    id: maxStripes
                    from: 1
                }
            }
        }
    }
}
