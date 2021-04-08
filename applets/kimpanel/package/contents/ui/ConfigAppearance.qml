/*
 *  Copyright 2014 Weng Xuetian <wengxt@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Dialogs 1.1 as QtDialogs
import QtQuick.Layouts 1.0

import org.kde.kirigami 2.5 as Kirigami
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Kirigami.FormLayout {
    id: iconsPage

    property alias cfg_vertical_lookup_table: verticalLookupTable.checked
    property bool cfg_use_default_font
    property font cfg_font
    property bool cfg_scaleIconsToFit

    QQC2.CheckBox {
        id: verticalLookupTable
        Kirigami.FormData.label: i18n("Input method list:")
        text: i18n("Vertical")
    }

    RowLayout {
        Kirigami.FormData.label: i18n("Font:")

        QQC2.CheckBox {
            id: useCustomFont
            checked: !cfg_use_default_font
            onClicked: cfg_use_default_font = !checked
            text: i18n("Use custom:")
        }

        QQC2.TextField {
            enabled: useCustomFont.checked
            readOnly: true
            text: i18nc("The selected font family and font size", font.family + " " + font.pointSize + "pt")
            font: cfg_font
            Layout.fillHeight: true
        }

        QQC2.Button {
            enabled: useCustomFont.checked
            icon.name: "document-edit"
            onClicked: fontDialog.open();

            QQC2.ToolTip {
                visible: parent.hovered
                text: i18n("Select Font...")
            }
        }
    }

    QQC2.RadioButton {
        Kirigami.FormData.label: i18nc("The arrangement of icons in the Panel", "Panel icon size:")
        text: i18n("Small")
        checked: cfg_scaleIconsToFit == false
        onToggled: cfg_scaleIconsToFit = !checked
    }
    QQC2.RadioButton {
        id: automaticRadioButton
        text: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? i18n("Scale with Panel height")
                                                                    : i18n("Scale with Panel width")
        checked: cfg_scaleIconsToFit == true
        onToggled: cfg_scaleIconsToFit = checked
    }

    QtDialogs.FontDialog {
        id: fontDialog
        title: i18nc("@title:window", "Select Font")

        font: !cfg_font || cfg_font.family === "" ? PlasmaCore.Theme.defaultFont : cfg_font

        onAccepted: {
            cfg_font = font
        }
    }
}
