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
import QtQuick.Controls 2.4 as Controls
import QtQuick.Layouts 1.3 as Layouts
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.2 as KCM

import org.kde.plasma.core 2.0 as PlasmaCore

KCM.SimpleKCM {
    id: root

    KCM.ConfigModule.buttons: KCM.ConfigModule.Help | KCM.ConfigModule.Defaults | KCM.ConfigModule.Apply

    implicitWidth: Kirigami.Units.gridUnit * 40

    Kirigami.FormLayout {
        id: formLayout

        // Visaul behavior settings
        Controls.CheckBox {
            id: showToolTips
            Kirigami.FormData.label: i18n("Visual behavior:")
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

        Item {
            Kirigami.FormData.isSection: false
        }

        // Click behavior settings

        Controls.RadioButton {
            id: singleClick
            Kirigami.FormData.label: i18n("Click behavior:")
            text: i18n("Single-click to open files and folders")
            checked: kcm.singleClick
            onCheckedChanged: kcm.singleClick = checked
        }

        Controls.RadioButton {
            id: doubleClick
            text: i18n("Double-click to open files and folders (single click to select)")
        }

        Connections {
            target: kcm
            onSingleClickChanged: {
                singleClick.checked = kcm.singleClick
                doubleClick.checked = !singleClick.checked
            }
        }
    }
}
