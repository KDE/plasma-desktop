/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *   Copyright (C) 2015 by Kai Uwe Broulik <kde@privat.broulik.de>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.4
import QtQuick.Controls 1.0
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

import org.kde.private.desktopcontainment.desktop 0.1 as Desktop
import org.kde.private.desktopcontainment.folder 0.1 as Folder

Item {
    id: configIcons

    width: childrenRect.width
    height: childrenRect.height

    property bool isPopup: (plasmoid.location != PlasmaCore.Types.Floating)

    property string cfg_icon: plasmoid.configuration.icon
    property alias cfg_useCustomIcon: useCustomIcon.checked
    property alias cfg_arrangement: arrangement.currentIndex
    property alias cfg_alignment: alignment.currentIndex
    property alias cfg_locked: locked.checked
    property alias cfg_sortMode: sortMode.mode
    property alias cfg_sortDesc: sortDesc.checked
    property alias cfg_sortDirsFirst: sortDirsFirst.checked
    property alias cfg_toolTips: toolTips.checked
    property alias cfg_selectionMarkers: selectionMarkers.checked
    property alias cfg_popups: popups.checked
    property alias cfg_previews: previews.checked
    property alias cfg_previewPlugins: previewPluginsDialog.previewPlugins
    property alias cfg_viewMode: viewMode.currentIndex
    property alias cfg_iconSize: iconSize.value
    property alias cfg_textLines: textLines.value

    IconDialog {
        id: iconDialog
        onIconNameChanged: cfg_icon = iconName || "folder"
    }

    GridLayout {

        // Row 0: "Panel button"
        Label {
            Layout.row: 0
            Layout.column: 0

            visible: isPopup
            text: i18n("Panel button:")
        }

        CheckBox {
            id: useCustomIcon
            Layout.row: 0
            Layout.column: 1
            Layout.columnSpan: 2

            visible: isPopup
            checked: cfg_useCustomIcon
            text: i18n("Use a custom icon")
        }

        RowLayout {
            Layout.row: 0
            Layout.column: 3
            Layout.alignment: Qt.AlignRight
            spacing: units.smallSpacing

            visible: isPopup

            Button {
                id: iconButton
                Layout.minimumWidth: units.iconSizes.large + units.smallSpacing * 2
                Layout.maximumWidth: Layout.minimumWidth
                Layout.minimumHeight: Layout.minimumWidth
                Layout.maximumHeight: Layout.minimumWidth

                checkable: true
                enabled: useCustomIcon.checked

                onClicked: {
                    checked = Qt.binding(function() {
                        return iconMenu.status === PlasmaComponents.DialogStatus.Open;
                    })

                    iconMenu.open(0, height);
                }

                PlasmaCore.IconItem {
                    anchors.centerIn: parent
                    width: units.iconSizes.large
                    height: width
                    source: cfg_icon
                }
            }

            PlasmaComponents.ContextMenu {
                id: iconMenu
                visualParent: iconButton

                PlasmaComponents.MenuItem {
                    text: i18nc("@item:inmenu Open icon chooser dialog", "Choose...")
                    icon: "document-open-folder"
                    onClicked: iconDialog.open()
                }

                PlasmaComponents.MenuItem {
                    text: i18nc("@item:inmenu Reset icon to default", "Clear Icon")
                    icon: "edit-clear"
                    onClicked: cfg_icon = "folder"
                }
            }
        }

        // Row 1: Spacing
        Item {
            Layout.row: 1
            Layout.column: 0
            Layout.minimumHeight: units.largeSpacing
            visible: isPopup
        }

        // Row 2: "Arrangment" - "Arrange in"
        Label {
            Layout.row: 2
            Layout.column: 0

            text: i18n("Arrangement:")
        }

        Label {
            Layout.row: 2
            Layout.column: 1

            text: i18n("Arrange in")
        }

        ComboBox {
            id: arrangement
            Layout.row: 2
            Layout.column: 2
            Layout.columnSpan: 2
            Layout.fillWidth: true

            model: [i18n("Rows"), i18n("Columns")]
        }

        // Row 3: "Arrangment" - "Align"
        Label {
            Layout.row: 3
            Layout.column: 1

            text: i18n("Align")
        }

        ComboBox {
            id: alignment
            Layout.row: 3
            Layout.column: 2
            Layout.columnSpan: 2
            Layout.fillWidth: true

            model: [i18n("Left"), i18n("Right")]
        }

        // Row 4: "Arrangment" - "Lock"
        CheckBox {
            id: locked
            Layout.row: 4
            Layout.column: 1
            Layout.columnSpan: 3

            visible: ("containmentType" in plasmoid)

            text: i18n("Lock in place")
        }

        // Row 5: Spacing
        Item {
            Layout.row: 5
            Layout.column: 0
            Layout.minimumHeight: units.largeSpacing
        }

        // Row 6: "Sorting" - "Sort by"
        Label {
            Layout.row: 6
            Layout.column: 0

            text: i18n("Sorting:")
        }

        Label {
            Layout.row: 6
            Layout.column: 1

            text: i18n("Sort by")
        }

        ComboBox {
            id: sortMode
            Layout.row: 6
            Layout.column: 2
            Layout.columnSpan: 2
            Layout.fillWidth: true

            property int mode
            // FIXME TODO HACK: This maps the combo box list model to the KDirModel::ModelColumns
            // enum, which should be done in C++.
            property variant indexToMode: [-1, 0, 1, 6, 2]
            property variant modeToIndex: {'-1' : '0', '0' : '1', '1' : '2', '6' : '3', '2' : '4'}

            model: [i18n("Unsorted"), i18n("Name"), i18n("Size"), i18n("Type"), i18n("Date")]

            Component.onCompleted: currentIndex = modeToIndex[mode]
            onActivated: mode = indexToMode[index]
        }

        // Row 7: "Sorting" - "Descending"
        CheckBox {
            id: sortDesc
            Layout.row: 7
            Layout.column: 1
            Layout.columnSpan: 3

            enabled: (sortMode.currentIndex != 0)

            text: i18n("Descending")
        }

        // Row 8: "Sorting" - "Directories first"
        CheckBox {
            id: sortDirsFirst
            Layout.row: 8
            Layout.column: 1
            Layout.columnSpan: 3

            enabled: (sortMode.currentIndex != 0)

            text: i18n("Folders first")
        }

        // Row 9: Spacing
        Item {
            Layout.row: 9
            Layout.column: 0
            Layout.minimumHeight: units.largeSpacing
        }

        // Row 10: "Appearance" - "View mode"
        Label {
            Layout.row: (isPopup ? 10 : 11)
            Layout.column: 0

            text: i18n("Appearance:")
        }

        Label {
            Layout.row: 10
            Layout.column: 1

            visible: isPopup

            text: i18nc("whether to use icon or list view", "View mode")
        }

        ComboBox {
            id: viewMode
            Layout.row: 10
            Layout.column: 2
            Layout.columnSpan: 2
            Layout.fillWidth: true

            visible: isPopup

            model: [i18n("List"), i18n("Icons")]
        }

        // Rows 11+12: "Appearance" - "Size"
        Label {
            Layout.row: 11
            Layout.column: 1

            visible: !isPopup || viewMode.currentIndex === 1

            text: i18n("Size")
        }

        Slider {
            id: iconSize
            Layout.row: 11
            Layout.column: 2
            Layout.columnSpan: 2
            Layout.fillWidth: true

            visible: !isPopup || viewMode.currentIndex === 1

            minimumValue: 0
            maximumValue: 5
            stepSize: 1
            tickmarksEnabled: true
        }

        Label {
            Layout.row: 12
            Layout.column: 2
            Layout.alignment: Qt.AlignLeft

            visible: !isPopup || viewMode.currentIndex === 1

            text: i18n("Small")
        }

        Label {
            Layout.row: 12
            Layout.column: 3
            Layout.alignment: Qt.AlignRight

            visible: !isPopup || viewMode.currentIndex === 1

            text: i18n("Large")
        }

        // Row 13: "Appearance" - "Text lines"
        Label {
            Layout.row: 13
            Layout.column: 1

            visible: !isPopup || viewMode.currentIndex === 1

            text: i18n("Text lines")
        }

        SpinBox {
            id: textLines
            Layout.row: 13
            Layout.column: 2
            Layout.columnSpan: 2

            visible: !isPopup || viewMode.currentIndex === 1

            minimumValue: 1
            maximumValue: 10
            stepSize: 1
        }

        // Row 14: Spacing
        Item {
            Layout.row: 14
            Layout.column: 0
            Layout.minimumHeight: units.largeSpacing
        }

        // Row 15: "Features" - "Tool tips"
        Label {
            Layout.row: 15
            Layout.column: 0

            text: i18n("Features:")
        }

        CheckBox {
            id: toolTips
            Layout.row: 15
            Layout.column: 1
            Layout.columnSpan: 3

            text: i18n("Tool tips")
        }

        // Row 16: "Features" - "Selection markers"
        CheckBox {
            id: selectionMarkers
            Layout.row: 16
            Layout.column: 1
            Layout.columnSpan: 3

            visible: Qt.styleHints.singleClickActivation

            text: i18n("Selection markers")
        }

        // Row 17: "Features" - "Folder preview popups"
        CheckBox {
            id: popups
            Layout.row: 17
            Layout.column: 1
            Layout.columnSpan: 3

            visible: !isPopup

            text: i18n("Folder preview popups")
        }

        // Rows 18+19: "Features" - "Preview thumbnails"
        CheckBox {
            id: previews
            Layout.row: 18
            Layout.column: 1
            Layout.columnSpan: 3

            text: i18n("Preview thumbnails")
        }

        Button {
            id: previewSettings
            Layout.row: 19
            Layout.column: 3
            Layout.alignment: Qt.AlignRight

            text: i18n("More Preview Options...")

            onClicked: {
                previewPluginsDialog.visible = true;
            }
        }
    }

    FolderItemPreviewPluginsDialog {
        id: previewPluginsDialog
    }
}
