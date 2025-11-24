/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs as QtDialogs
import QtQuick.Layouts
import org.kde.plasma.plasmoid

import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property bool cfg_vertical_lookup_table
    property bool cfg_use_default_font
    property font cfg_font
    property bool cfg_scaleIconsToFit

    Kirigami.FormLayout {
        QQC2.ButtonGroup { id: layoutRadioGroup }
        QQC2.ButtonGroup { id: scaleRadioGroup }

        QQC2.RadioButton {
            id: verticalLayoutRadioButton
            Kirigami.FormData.label: i18nc("@title:group for radiobuttons", "Input method list:") // qmllint disable unqualified
            text: i18nc("@option:radio IM list orientation", "Vertical") // qmllint disable unqualified
            checked: root.cfg_vertical_lookup_table == true
            onToggled: root.cfg_vertical_lookup_table = checked
            QQC2.ButtonGroup.group: layoutRadioGroup
        }
        QQC2.RadioButton {
            text: i18nc("@option:radio IM list orientation", "Horizontal") // qmllint disable unqualified
            checked: root.cfg_vertical_lookup_table == false
            onToggled: root.cfg_vertical_lookup_table = !checked
            QQC2.ButtonGroup.group: layoutRadioGroup
        }

        RowLayout {
            Kirigami.FormData.label: i18nc("@title group font selection", "Font:") // qmllint disable unqualified

            QQC2.CheckBox {
                id: useCustomFont
                checked: !root.cfg_use_default_font
                onClicked: root.cfg_use_default_font = !checked
                text: i18nc("@option:check use custom font", "Use custom:") // qmllint disable unqualified
            }

            QQC2.TextField {
                enabled: useCustomFont.checked
                readOnly: true
                text: i18nc("@info The selected font family (%1) and font size (%2)", "%1 %2 pt", font.family, font.pointSize) // qmllint disable unqualified
                font: root.cfg_font
                Layout.fillHeight: true
            }

            QQC2.Button {
                id: chooseFontButton
                enabled: useCustomFont.checked
                text: i18nc("@action:button opens font dialog", "Choose Font…") // qmllint disable unqualified
                display: QQC2.AbstractButton.IconOnly
                icon.name: "document-edit"
                onClicked: fontDialog.open();

                QQC2.ToolTip {
                    visible: chooseFontButton.hovered
                    text: chooseFontButton.text
                }
            }
        }

        QQC2.RadioButton {
            Kirigami.FormData.label: i18nc("@title:group The arrangement of icons in the Panel", "Panel icon size:") // qmllint disable unqualified
            text: i18nc("@option:radio panel icon size", "Small") // qmllint disable unqualified
            checked: root.cfg_scaleIconsToFit == false
            onToggled: root.cfg_scaleIconsToFit = !checked
            QQC2.ButtonGroup.group: scaleRadioGroup
        }
        QQC2.RadioButton {
            id: automaticScaleRadioButton
            text: Plasmoid.formFactor === PlasmaCore.Types.Horizontal
                ? i18nc("@option:radio panel icon size", "Scale with Panel height") // qmllint disable unqualified
                : i18nc("@option:radio panel icon size", "Scale with Panel width") // qmllint disable unqualified
            checked: root.cfg_scaleIconsToFit == true
            onToggled: root.cfg_scaleIconsToFit = checked
            QQC2.ButtonGroup.group: scaleRadioGroup
        }

        QtDialogs.FontDialog {
            id: fontDialog
            title: i18nc("@title:window", "Select Font") // qmllint disable unqualified

            selectedFont: !root.cfg_font || root.cfg_font.family === "" ? Kirigami.Theme.defaultFont : root.cfg_font

            onAccepted: {
                root.cfg_font = selectedFont
            }
        }
    }
}
