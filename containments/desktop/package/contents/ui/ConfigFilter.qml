/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.core as PlasmaCore
import org.kde.private.desktopcontainment.folder 0.1 as Folder
import org.kde.kitemmodels as KItemModels

ColumnLayout {
    id: configIcons

    property alias cfg_filterMode: filterMode.currentIndex
    property alias cfg_filterPattern: filterPattern.text
    property alias cfg_filterMimeTypes: mimeTypesModel.checkedTypes
    property alias cfg_showHiddenFiles: showHiddenFiles.checked

    KItemModels.KSortFilterProxyModel {
        id: filteredMimeTypesModel

        sourceModel: Folder.MimeTypesModel {
            id: mimeTypesModel
        }

        // SortFilterModel doesn't have a case-sensitivity option
        // but filterRegExp always causes case-insensitive sorting.
        filterRegularExpression: RegExp(mimeFilter.text)
        filterRoleName: "name"

        sortRoleName: "name"
        sortOrder: Qt.AscendingOrder

        function checkFiltered() {
            const types = [];

            const nameRole = KItemModels.KRoleNames.role("name");
            const n = count;
            for (let i = 0; i < n; ++i) {
                const name = data(index(i, 0), nameRole);
                types.push(name);
            }
            mimeTypesModel.checkedTypes = types;
        }

        function uncheckFiltered() {
            mimeTypesModel.checkedTypes = [];
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

        ScrollView {
            id: scrollView
            enabled: (filterMode.currentIndex > 0)
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 100 // Note: this arbitrary number is a workaround to the layout trying to resize the scrollview to an huge value, the behavior is still correctly stretching

            Component.onCompleted: {
                if (background) {
                    background.visible = true;
                }
            }

            contentItem: ListView {
                id: mimeTypesView
                clip: true
                reuseItems: true
                keyNavigationEnabled: true
                currentIndex: -1

                model: filteredMimeTypesModel
                property real columnSize: Kirigami.Units.gridUnit * 15
                headerPositioning: ListView.OverlayHeader

                header: HorizontalHeaderView {
                    id: headerView
                    z: 9
                    implicitWidth: mimeTypesView.width
                    rowHeightProvider: function () {
                        return Kirigami.Units.gridUnit * 2
                    }
                    clip: true // This removes event handling blocking by the header
                    model: ListModel {
                        Component.onCompleted: {
                            append({ display: i18n("File Type") });
                            append({ display: i18n("Description") });
                        }
                    }
                    interactive: false
                    columnWidthProvider: function(column) {
                        if (column === 0) {
                            return mimeTypesView.columnSize;
                        } else {
                            return mimeTypesView.width - mimeTypesView.columnSize;
                        }
                    }
                }

                delegate: CheckDelegate {
                    id: delegate

                    required property int index
                    required property string name
                    required property string comment
                    required property string decoration

                    width: mimeTypesView.width
                    height: Kirigami.Units.iconSizes.small + padding * 2

                    icon.name: decoration

                    checked: mimeTypesModel.checkedTypes.indexOf(name) >= 0
                    highlighted: ListView.isCurrentItem && activeFocus

                    onToggled: {
                        ListView.view.currentIndex = index;
                        const idx = mimeTypesModel.checkedTypes.indexOf(name);
                        if (idx >= 0) {
                            mimeTypesModel.checkedTypes.splice(idx, 1);
                        } else {
                            mimeTypesModel.checkedTypes.push(name)
                        }
                    }

                    contentItem: RowLayout {
                        spacing: Kirigami.Units.smallSpacing

                        RowLayout {
                            // Note: assuming check indicator will be on the left
                            Layout.preferredWidth: mimeTypesView.columnSize - (delegate.mirrored ? delegate.rightPadding : delegate.leftPadding)
                            Layout.minimumWidth: Layout.preferredWidth
                            Layout.maximumWidth: Layout.preferredWidth
                            Layout.fillWidth: false
                            Layout.fillHeight: true

                            spacing: delegate.spacing

                            Kirigami.Icon {
                                visible: delegate.icon.name !== ""
                                source: delegate.icon.name
                                animated: false
                                selected: (delegate.pressed || delegate.highlighted)
                                Layout.alignment: Qt.AlignVCenter
                                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                            }

                            Label {
                                text: delegate.name
                                elide: Text.ElideRight
                                color: (delegate.pressed || delegate.highlighted) ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                            }
                        }
                        Label {
                            text: delegate.comment
                            elide: Text.ElideRight
                            color: (delegate.pressed || delegate.highlighted) ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }
                    }
                }
            }
        }

        RowLayout {
            id: selectLayout
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

            Button {
                enabled: (filterMode.currentIndex > 0)
                icon.name: filteredMimeTypesModel.sortOrder === Qt.AscendingOrder ? "view-sort-ascending-symbolic" : "view-sort-descending-symbolic"
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.visible: (Kirigami.Settings.isMobile ? pressed : hovered) && ToolTip.text.length > 0
                ToolTip.text: i18n("Switch Sort Order")
                onClicked: {
                    filteredMimeTypesModel.sortOrder = filteredMimeTypesModel.sortOrder === Qt.AscendingOrder ? Qt.DescendingOrder : Qt.AscendingOrder;
                    filteredMimeTypesModel.sort(0, filteredMimeTypesModel.sortOrder);
                }
            }
        }
    }
}
