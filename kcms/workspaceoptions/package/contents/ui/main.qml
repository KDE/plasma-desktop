/*
 * Copyright 2018 Furkan Tokac <furkantokac34@gmail.com>
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
import QtQuick.Controls 1.4 as Controls
import QtQuick.Layouts 1.3 as Layouts
import QtQuick.Controls.Styles 1.4 as Styles
import org.kde.kcm 1.1 as KCM

import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root

    Controls.ScrollView {
        anchors.fill: parent

        Layouts.ColumnLayout {
            id: maincol
            spacing: units.largeSpacing

            // General Settings
            Column {
                spacing: units.smallSpacing
                leftPadding: units.smallSpacing

                Controls.Label {
                    text: i18n("General Settings")
                }

                Controls.CheckBox {
                    id: showToolTips
                    text: i18n("Display informational tooltips on mouse hover")
                    checked: kcm.toolTip
                    onCheckedChanged: kcm.toolTip = checked
                }

                Controls.CheckBox {
                    id: showVisualFeedback
                    text: i18n("Display visual feedback for status changes")
                    checked: kcm.visualFeedback
                    onCheckedChanged: kcm.visualFeedback = checked
                }

                Connections {
                    target: kcm
                    onToolTipChanged: showToolTips.checked = kcm.toolTip
                    onVisualFeedbackChanged: showVisualFeedback.checked = kcm.visualFeedback
                }
            }

            // ClickBehaviour Settings
            Column {
                spacing: units.smallSpacing
                leftPadding: units.smallSpacing
                Controls.ExclusiveGroup { id: clickBehaviourGroup }

                Controls.Label {
                    text: i18n("Click Behaviour")
                }

                Controls.RadioButton {
                    id: singleClick
                    text: i18n("Single-click to open files and folders")
                    checked: kcm.singleClick
                    exclusiveGroup: clickBehaviourGroup
                    onCheckedChanged: kcm.singleClick = checked
                }

                Controls.RadioButton {
                    id: doubleClick
                    text: i18n("Double-click to open files and folders (select icons on first click)")
                    exclusiveGroup: clickBehaviourGroup
                }

                Connections {
                    target: kcm
                    onSingleClickChanged: {
                        singleClick.checked = kcm.singleClick
                        doubleClick.checked = !singleClick.checked
                    }
                }
            }
        } // END Layouts.ColumnLayout
    } // END Controls.ScrollView
} // END Item
