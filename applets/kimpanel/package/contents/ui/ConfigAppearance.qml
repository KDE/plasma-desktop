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

import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls
import QtQuick.Dialogs 1.1 as QtDialogs
import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: iconsPage
    implicitWidth: pageColumn.implicitWidth
    implicitHeight: pageColumn.implicitHeight

    property alias cfg_vertical_lookup_table: verticalLookupTable.checked
    property alias cfg_use_default_font: useDefaultFont.checked
    property font cfg_font

    onCfg_fontChanged: {
        if (cfg_font.family == '') {
            cfg_font = theme.defaultFont
        }
        fontDialog.font = cfg_font
    }

    ColumnLayout {
        id: pageColumn
        width: parent.width

        QtControls.CheckBox {
            id: verticalLookupTable
            text: i18n("Vertical List")
        }

        QtControls.CheckBox {
            id: useDefaultFont
            text: i18n("Use Default Font")
        }

        RowLayout {
            width: parent.width

            QtControls.Label {
                text: i18n("Custom Font:")
            }

            QtControls.TextField {
                id: fontPreviewLabel
                Layout.fillWidth: true
                enabled: !cfg_use_default_font
                anchors.verticalCenter: parent.verticalCenter
                readOnly: true
                font: cfg_font
                text: cfg_font.family + ' ' + cfg_font.pointSize
            }

            QtControls.Button {
                id: fontButton
                enabled: !cfg_use_default_font
                text: i18n("Select Font")
                onClicked: fontDialog.open();
            }
        }
    }

    QtDialogs.FontDialog {
        id: fontDialog
        title: i18nc("@title:window", "Select Font")

        onAccepted: {
            cfg_font = font
        }
    }
}
