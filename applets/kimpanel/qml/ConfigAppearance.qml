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

    Kirigami.Form {
        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: layoutRadioGroup }

            Kirigami.FormEntry {
                title: i18nc("@title:group for radiobuttons", "Input method list:")
                contentItem: QQC2.RadioButton {
                    id: verticalLayoutRadioButton
                    text: i18nc("@option:radio IM list orientation", "Vertical")
                    checked: root.cfg_vertical_lookup_table == true
                    onToggled: root.cfg_vertical_lookup_table = checked
                    QQC2.ButtonGroup.group: layoutRadioGroup
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio IM list orientation", "Horizontal")
                    checked: root.cfg_vertical_lookup_table == false
                    onToggled: root.cfg_vertical_lookup_table = !checked
                    QQC2.ButtonGroup.group: layoutRadioGroup
                }
            }
        }
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@title group font selection", "Font:")
                contentItem: QQC2.CheckBox {
                    id: useCustomFont
                    checked: !root.cfg_use_default_font
                    onClicked: root.cfg_use_default_font = !checked
                    text: i18nc("@option:check use custom font", "Use custom:")
                }
                trailingItems: [
                    QQC2.TextField {
                        id: demoTextField
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.smallSpacing
                        enabled: useCustomFont.checked
                        readOnly: true
                        text: i18nc("@info The selected font family (%1) and font size (%2)", "%1 %2 pt", font.family, font.pointSize)
                        font: !root.cfg_font || root.cfg_font.family === "" ? Kirigami.Theme.defaultFont : root.cfg_font
                        Layout.fillHeight: true
                    },

                    QQC2.Button {
                        id: chooseFontButton
                        enabled: useCustomFont.checked
                        text: i18nc("@action:button opens font dialog", "Choose Font…")
                        display: QQC2.AbstractButton.IconOnly
                        icon.name: "document-edit"
                        onClicked: {
                            fontDialog.open();
                            fontDialog.selectedFont = demoTextField.font
                        }

                        QQC2.ToolTip.visible: hovered || visualFocus
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

                    }
                ]
            }
        }
        Kirigami.FormGroup {
            QQC2.ButtonGroup { id: scaleRadioGroup }
            Kirigami.FormEntry {
                title: i18nc("@title:group The arrangement of icons in the Panel", "Panel icon size:")
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio panel icon size", "Small")
                    checked: root.cfg_scaleIconsToFit == false
                    onToggled: root.cfg_scaleIconsToFit = !checked
                    QQC2.ButtonGroup.group: scaleRadioGroup
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    id: automaticScaleRadioButton
                    text: Plasmoid.formFactor === PlasmaCore.Types.Horizontal
                        ? i18nc("@option:radio panel icon size", "Scale with Panel height")
                        : i18nc("@option:radio panel icon size", "Scale with Panel width")
                    checked: root.cfg_scaleIconsToFit == true
                    onToggled: root.cfg_scaleIconsToFit = checked
                    QQC2.ButtonGroup.group: scaleRadioGroup
                }
            }
        }
    }

    QtDialogs.FontDialog {
        id: fontDialog
        title: i18nc("@title:window", "Select Font")

        selectedFont: !root.cfg_font || root.cfg_font.family === "" ? Kirigami.Theme.defaultFont : root.cfg_font

        onAccepted: {
            root.cfg_font = selectedFont
        }
    }
}
