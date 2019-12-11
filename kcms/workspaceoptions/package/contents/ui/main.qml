/*
 * Copyright 2018 Furkan Tokac <furkantokac34@gmail.com>
 * Copyright (C) 2019 Nate Graham <nate@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
import QtQuick 2.7
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcm 1.2 as KCM

import org.kde.plasma.core 2.0 as PlasmaCore

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
            enabled: !kcm.plasmaSettings.isImmutable("delay")
            checked: kcm.plasmaSettings.delay > 0
            onCheckedChanged: {
                if (checked) {
                    kcm.plasmaSettings.delay = 0.7
                } else {
                    kcm.plasmaSettings.delay = -1
                }
            }
        }

        QQC2.CheckBox {
            id: showVisualFeedback
            text: i18n("Display visual feedback for status changes")
            enabled: !kcm.plasmaSettings.isImmutable("osdEnabled")
            checked: kcm.plasmaSettings.osdEnabled
            onCheckedChanged: kcm.plasmaSettings.osdEnabled = checked
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // We want to show the slider in a logarithmic way. ie
        // move from 4x, 3x, 2x, 1x, 0.5x, 0.25x, 0.125x
        // 0 is a special case
        ColumnLayout {
            enabled: !kcm.globalsSettings.isImmutable("animationDurationFactor")
            Kirigami.FormData.label: i18n("Animation speed:")
            Kirigami.FormData.buddyFor: slider

            QQC2.Slider {
                id: slider
                Layout.fillWidth: true
                from: -4
                to: 4
                stepSize: 1
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
            Kirigami.FormData.label: i18n("Click behavior:")
            text: i18n("Single-click to open files and folders")
            enabled: !kcm.globalsSettings.isImmutable("singleClick")
            checked: kcm.globalsSettings.singleClick
            onToggled: kcm.globalsSettings.singleClick = true
            QQC2.ButtonGroup.group: singleClickGroup
        }

        QQC2.RadioButton {
            id: doubleClick
            text: i18n("Double-click to open files and folders")
            enabled: !kcm.globalsSettings.isImmutable("singleClick")
            checked: !kcm.globalsSettings.singleClick
            onToggled: kcm.globalsSettings.singleClick = false
            QQC2.ButtonGroup.group: singleClickGroup
        }

        QQC2.Label {
            Layout.fillWidth: true
            text: singleClick.checked ? i18n("Select by clicking on item's selection marker") : i18n("Select by single-clicking")
            elide: Text.ElideRight
            font.pointSize: theme.smallestFont.pointSize
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
            enabled: !kcm.globalsSettings.isImmutable("scrollbarLeftClickNavigatesByPage")
            checked: kcm.globalsSettings.scrollbarLeftClickNavigatesByPage
            onToggled: kcm.globalsSettings.scrollbarLeftClickNavigatesByPage = true
            QQC2.ButtonGroup.group: scrollHandleBehaviorGroup
        }

        QQC2.RadioButton {
            id: scrollBarLeftClickWarpsScrollHandle
            text: i18nc("@radio part of a complete sentence: 'Clicking in scrollbar track scrolls to the clicked location'", "Scrolls to the clicked location")
            enabled: !kcm.globalsSettings.isImmutable("scrollbarLeftClickNavigatesByPage")
            checked: !kcm.globalsSettings.scrollbarLeftClickNavigatesByPage
            onToggled: kcm.globalsSettings.scrollbarLeftClickNavigatesByPage = false
            QQC2.ButtonGroup.group: scrollHandleBehaviorGroup
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
            font.pointSize: theme.smallestFont.pointSize
        }
    }
}
