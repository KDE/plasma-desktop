/*
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.7
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcm 1.3 as KCM

KCM.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 40

    Kirigami.FormLayout {
        id: formLayout

        // Visual behavior settings
        QQC2.CheckBox {
            id: showToolTips
            Kirigami.FormData.label: i18n("Visual behavior:")
            text: i18n("Display informational tooltips on mouse hover")
            checked: kcm.plasmaSettings.delay > 0
            onCheckedChanged: {
                if (checked) {
                    kcm.plasmaSettings.delay = 0.7
                } else {
                    kcm.plasmaSettings.delay = -1
                }
            }

            KCM.SettingStateBinding {
                configObject: kcm.plasmaSettings
                settingName: "delay"
            }
        }

        QQC2.CheckBox {
            id: showVisualFeedback
            text: i18n("Display visual feedback for status changes")
            checked: kcm.plasmaSettings.osdEnabled
            onCheckedChanged: kcm.plasmaSettings.osdEnabled = checked

            KCM.SettingStateBinding {
                configObject: kcm.plasmaSettings
                settingName: "osdEnabled"
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // We want to show the slider in a logarithmic way. ie
        // move from 4x, 3x, 2x, 1x, 0.5x, 0.25x, 0.125x
        // 0 is a special case
        ColumnLayout {
            Kirigami.FormData.label: i18n("Animation speed:")
            Kirigami.FormData.buddyFor: slider

            QQC2.Slider {
                id: slider
                Layout.fillWidth: true
                from: -4
                to: 4
                stepSize: 0.5
                snapMode: QQC2.Slider.SnapAlways
                onMoved: {
                    if(value === to) {
                        kcm.globalsSettings.animationDurationFactor = 0;
                    } else {
                        kcm.globalsSettings.animationDurationFactor = 1.0 / Math.pow(2, value);
                    }
                }
                value: if (kcm.globalsSettings.animationDurationFactor === 0) {
                    return slider.to;
                } else {
                    return -(Math.log(kcm.globalsSettings.animationDurationFactor) / Math.log(2));
                }

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "animationDurationFactor"
                }
            }
            RowLayout {
                QQC2.Label {
                    text: i18nc("Animation speed", "Slow")
                }
                Item {
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: i18nc("Animation speed", "Instant")
                }
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // Click behavior settings

        QQC2.ButtonGroup { id: singleClickGroup }

        QQC2.RadioButton {
            id: singleClick
            Kirigami.FormData.label: i18nc("part of a sentence: 'Clicking files or folders [opens them/selects them]'", "Clicking files or folders:")
            text: i18nc("part of a sentence: 'Clicking files or folders opens them'", "Opens them")
            checked: kcm.globalsSettings.singleClick
            onToggled: kcm.globalsSettings.singleClick = true
            QQC2.ButtonGroup.group: singleClickGroup

            KCM.SettingStateBinding {
                configObject: kcm.globalsSettings
                settingName: "singleClick"
            }
        }

        QQC2.RadioButton {
            id: doubleClick
            text: i18nc("part of a sentence: 'Clicking files or folders selects them'", "Selects them")
            checked: !kcm.globalsSettings.singleClick
            onToggled: kcm.globalsSettings.singleClick = false
            QQC2.ButtonGroup.group: singleClickGroup

            KCM.SettingStateBinding {
                configObject: kcm.globalsSettings
                settingName: "singleClick"
                extraEnabledConditions: singleClick.enabled
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            Layout.minimumWidth: Kirigami.Units.gridUnit * 15
            text: singleClick.checked ? i18n("Select by clicking on item's selection marker") : i18n("Open by double-clicking instead")
            elide: Text.ElideRight
            font: Kirigami.Theme.smallFont
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // scroll handle settings

        QQC2.ButtonGroup { id: scrollHandleBehaviorGroup }

        QQC2.RadioButton {
            id: scrollbarLeftClickNavigatesByPage
            Kirigami.FormData.label: i18n("Clicking in scrollbar track:")
            text: i18nc("@radio part of a complete sentence: 'Clicking in scrollbar track scrolls one page up or down'", "Scrolls one page up or down")
            checked: kcm.globalsSettings.scrollbarLeftClickNavigatesByPage
            onToggled: kcm.globalsSettings.scrollbarLeftClickNavigatesByPage = true
            QQC2.ButtonGroup.group: scrollHandleBehaviorGroup

            KCM.SettingStateBinding {
                configObject: kcm.globalsSettings
                settingName: "scrollbarLeftClickNavigatesByPage"
            }
        }

        QQC2.RadioButton {
            id: scrollBarLeftClickWarpsScrollHandle
            text: i18nc("@radio part of a complete sentence: 'Clicking in scrollbar track scrolls to the clicked location'", "Scrolls to the clicked location")
            checked: !kcm.globalsSettings.scrollbarLeftClickNavigatesByPage
            onToggled: kcm.globalsSettings.scrollbarLeftClickNavigatesByPage = false
            QQC2.ButtonGroup.group: scrollHandleBehaviorGroup

            KCM.SettingStateBinding {
                configObject: kcm.globalsSettings
                settingName: "scrollbarLeftClickNavigatesByPage"
                extraEnabledConditions: scrollbarLeftClickNavigatesByPage.enabled
            }
        }

        // Don't show a label for what middle-clicking does when using the
        // "click to zoom the handle" behavior because Qt doesn't invert the
        // middle-click functionality when using this; see
        // https://bugreports.qt.io/browse/QTBUG-80728
        QQC2.Label {
            Layout.fillWidth: true
            visible: scrollbarLeftClickNavigatesByPage.checked
            text: i18n("Middle-click to scroll to clicked location")
            elide: Text.ElideRight
            font: Kirigami.Theme.smallFont
        }
    }
}
