/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls 1.0 as QQC1
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.core 2.1 as PlasmaCore

import org.kde.private.desktopcontainment.folder 0.1 as Folder

ColumnLayout {
    id: configIcons

    property alias cfg_filterMode: filterMode.currentIndex
    property alias cfg_filterPattern: filterPattern.text
    property alias cfg_filterMimeTypes: mimeTypesModel.checkedTypes
    property alias cfg_showHiddenFiles: showHiddenFiles.checked

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

            mimeTypesModel.checkedTypes = mimeTypesModel.checkedTypes
                .filter(x => types.indexOf(x) === -1);
        }
    }
    Kirigami.FormLayout {
        ComboBox {
            id: filterMode
            Kirigami.FormData.label: i18n("Files:")
            model: [i18n("Show all"), i18n("Show matching"), i18n("Hide matching")]
        }

        TextField {
            id: filterPattern
            Kirigami.FormData.label: i18n("File name pattern:")
            enabled: (filterMode.currentIndex > 0)
            inputMethodHints: Qt.ImhNoPredictiveText
        }

        Kirigami.SearchField {
            id: mimeFilter
            Kirigami.FormData.label: i18n("File types:")
            enabled: (filterMode.currentIndex > 0)
        }

        CheckBox {
            id: showHiddenFiles
            Kirigami.FormData.label: i18n("Show hidden files:")
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true

        CheckBox { // Purely for metrics.
            id: metricsCheckBox
            visible: false
        }

        QQC1.TableView {
            id: mimeTypesView

            // Signal the delegates listen to when user presses space to toggle current row.
            signal toggleCurrent()

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

            QQC1.TableViewColumn {
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
                        checked = Qt.binding(() => styleData.value);
                    }

                    Connections {
                        target: mimeTypesView
                        function onToggleCurrent() {
                            if (styleData.row === mimeTypesView.currentRow) {
                                model.checked = !checkBox.checked;
                            }
                        }
                    }
                }
            }

            QQC1.TableViewColumn {
                role: "decoration"
                width: PlasmaCore.Units.iconSizes.small
                resizable: false
                movable: false

                delegate: PlasmaCore.IconItem {
                    width: PlasmaCore.Units.iconSizes.small
                    height: PlasmaCore.Units.iconSizes.small
                    animated: false // TableView re-uses delegates, avoid animation when sorting/filtering.
                    source: styleData.value
                }
            }

            QQC1.TableViewColumn {
                id: nameColumn
                role: "name"
                title: i18n("File type")
                width: PlasmaCore.Units.gridUnit * 10 // Assume somewhat reasonable default for mime type name.
                onWidthChanged: mimeTypesView.adjustColumns()
                movable: false
            }

            QQC1.TableViewColumn {
                id: descriptionColumn
                role: "comment"
                title: i18n("Description")
                movable: false
                resizable: false
            }
        }

        RowLayout {
            Button {
                id: selectAllButton
                enabled: (filterMode.currentIndex > 0)
                icon.name: "edit-select-all"
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.visible: (Kirigami.Settings.isMobile ? pressed : hovered) && ToolTip.text.length > 0
                ToolTip.text: i18n("Select All")
                onClicked: filteredMimeTypesModel.checkFiltered()
            }

            Button {
                id: deselectAllButton
                enabled: (filterMode.currentIndex > 0)
                icon.name: "edit-select-none"
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.visible: (Kirigami.Settings.isMobile ? pressed : hovered) && ToolTip.text.length > 0
                ToolTip.text: i18n("Deselect All")
                onClicked: filteredMimeTypesModel.uncheckFiltered()
            }
        }
    }
}
