/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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

import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.private.desktopcontainment.folder 0.1 as Folder

Item {
    id: configIcons

    width: childrenRect.width
    height: childrenRect.height

    property alias cfg_filterMode: filterMode.currentIndex
    property alias cfg_filterPattern: filterPattern.text
    property alias cfg_filterMimeTypes: mimeTypesModel.checkedTypes

    PlasmaCore.SortFilterModel {
        id: filteredMimeTypesModel

        sourceModel: Folder.MimeTypesModel {
            id: mimeTypesModel
        }

        // SortFilterModel doesn't have a case-sensitivity option
        // but filterRegExp always causes case-insensitive sorting.
        filterRegExp: mimeFilter.text
        filterRole: "name"

        sortRole: mimeTypesView.getColumn(mimeTypesView.sortIndicatorColumn).role
        sortOrder: mimeTypesView.sortIndicatorOrder

        function checkFiltered() {
            var types = [];

            for (var i = 0; i < count; ++i) {
                types.push(get(i).name);
            }

            mimeTypesModel.checkedTypes = types;
        }

        function uncheckFiltered() {
            var types = [];

            for (var i = 0; i < count; ++i) {
                types.push(get(i).name);
            }

            mimeTypesModel.checkedTypes = mimeTypesModel.checkedTypes.filter(function(x) {
                return types.indexOf(x) < 0; });
        }
    }

    ColumnLayout {
        width: parent.width
        height: parent.height

        ComboBox {
            id: filterMode

            Layout.fillWidth: true

            model: [i18n("Show All Files"), i18n("Show Files Matching"), i18n("Hide Files Matching")]
        }

        Label {
            Layout.fillWidth: true

            text: i18n("File name pattern:")
        }

        TextField {
            id: filterPattern

            Layout.fillWidth: true

            enabled: (filterMode.currentIndex > 0)
        }

        Label {
            Layout.fillWidth: true

            text: i18n("File types:")
        }

        TextField {
            id: mimeFilter

            Layout.fillWidth: true

            enabled: (filterMode.currentIndex > 0)

            placeholderText: i18n("Search file type...")
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            CheckBox { // Purely for metrics.
                id: metricsCheckBox
                visible: false
            }

            TableView {
                id: mimeTypesView

                // Signal the delegates listen to when user presses space to toggle current row.
                signal toggleCurrent

                Layout.fillWidth: true
                Layout.fillHeight: true

                enabled: (filterMode.currentIndex > 0)

                model: filteredMimeTypesModel

                sortIndicatorVisible: true
                sortIndicatorColumn: 2 // Default to sort by "File type".

                onSortIndicatorColumnChanged: { // Disallow sorting by icon.
                    if (sortIndicatorColumn === 1) {
                        sortIndicatorColumn = 2;
                    }
                }

                Keys.onSpacePressed: toggleCurrent()

                function adjustColumns() {
                    // Resize description column to take whatever space is left.
                    var width = viewport.width;
                    for (var i = 0; i < columnCount - 1; ++i) {
                        width -= getColumn(i).width;
                    }
                    descriptionColumn.width = width;
                }

                onWidthChanged: adjustColumns()
                // Component.onCompleted is too early to do this...
                onRowCountChanged: adjustColumns()

                TableViewColumn {
                    role: "checked"
                    width: metricsCheckBox.width
                    resizable: false
                    movable: false

                    delegate: CheckBox {
                        id: checkBox

                        checked: styleData.value
                        activeFocusOnTab: false // only let the TableView as a whole get focus
                        onClicked: {
                            model.checked = checked
                            // Clicking it breaks the binding to the model value which becomes
                            // an issue during sorting as TableView re-uses delegates.
                            checked = Qt.binding(function() {
                                return styleData.value;
                            });
                        }

                        Connections {
                            target: mimeTypesView
                            onToggleCurrent: {
                                if (styleData.row === mimeTypesView.currentRow) {
                                    model.checked = !checkBox.checked
                                }
                            }
                        }
                    }
                }

                TableViewColumn {
                    role: "decoration"
                    width: units.iconSizes.small
                    resizable: false
                    movable: false

                    delegate: PlasmaCore.IconItem {
                        width: units.iconSizes.small
                        height: units.iconSizes.small
                        animated: false // TableView re-uses delegates, avoid animation when sorting/filtering.
                        source: styleData.value
                    }
                }

                TableViewColumn {
                    id: nameColumn
                    role: "name"
                    title: i18n("File type")
                    width: units.gridUnit * 10 // Assume somewhat reasonable default for mime type name.
                    onWidthChanged: mimeTypesView.adjustColumns()
                    movable: false
                }
                TableViewColumn {
                    id: descriptionColumn
                    role: "comment"
                    title: i18n("Description")
                    movable: false
                    resizable: false
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                // Need to explicitly base the size off the button's implicitWidth
                // to avoid the column from growing way too wide due to fillWidth...
                Layout.maximumWidth: Math.max(selectAllButton.implicitWidth, deselectAllButton.implicitWidth)

                Button {
                    id: selectAllButton
                    Layout.fillWidth: true

                    enabled: (filterMode.currentIndex > 0)

                    text: i18n("Select All")

                    onClicked: filteredMimeTypesModel.checkFiltered()
                }

                Button {
                    id: deselectAllButton
                    Layout.fillWidth: true

                    enabled: (filterMode.currentIndex > 0)

                    text: i18n("Deselect All")

                    onClicked: filteredMimeTypesModel.uncheckFiltered()
                }
            }
        }
    }
}
