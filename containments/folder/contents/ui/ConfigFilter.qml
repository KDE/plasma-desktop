/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.private.desktopcontainment.folder as Folder
import org.kde.kitemmodels as KItemModels
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
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

        filterRegularExpression: RegExp(mimeFilter.text, "i")
        filterRoleName: "name"

        sortRoleName: "name"
        sortOrder: Qt.AscendingOrder

        function checkFiltered() {
            var types = [];

            for (var i = 0; i < count; ++i) {
                types.push(index(i, 0).data(Qt.UserRole));
            }

            mimeTypesModel.checkedTypes = types;
        }

        function uncheckFiltered() {
            var types = [];

            for (var i = 0; i < count; ++i) {
                types.push(index(i, 0).data(Qt.UserRole));
            }

            mimeTypesModel.checkedTypes = mimeTypesModel.checkedTypes
                .filter(x => types.indexOf(x) === -1);
        }
    }

    header: Kirigami.FormLayout {
        ComboBox {
            id: filterMode
            Kirigami.FormData.label: i18nc("@label:listbox filter mode", "Files:")
            model: [i18nc("@item:inlistbox filter mode", "Show all"), i18nc("@item:inlistbox filter mode", "Show matching"), i18nc("@item:inlistbox filter mode", "Hide matching")]
        }

        TextField {
            id: filterPattern
            Kirigami.FormData.label: i18nc("@label:textbox", "File name pattern:")
            enabled: (filterMode.currentIndex > 0)
            inputMethodHints: Qt.ImhNoPredictiveText
        }

        Kirigami.SearchField {
            id: mimeFilter
            Kirigami.FormData.label: i18nc("@label:textbox filter list by", "File types:")
            enabled: (filterMode.currentIndex > 0)
        }

        CheckBox {
            id: showHiddenFiles
            Kirigami.FormData.label: i18nc("@option:check prefix", "Show hidden files:")
        }
    }

    view: ListView {
        id: mimeTypesView
        clip: true
        enabled: (filterMode.currentIndex > 0)

        // Signal the delegates listen to when user presses space to toggle current row.
        signal toggleCurrent()


        model: filteredMimeTypesModel
        property real columnSize: Kirigami.Units.gridUnit * 15
        headerPositioning: ListView.OverlayHeader

        Keys.onSpacePressed: toggleCurrent()

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
                    append({ display: i18nc("@title:column", "File Type") });
                    append({ display: i18nc("@title:column", "Description") });
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

        delegate: ItemDelegate {
            id: delegate
            width: mimeTypesView.width
            required property string name
            required property string comment
            required property var decoration

            contentItem: RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.iconSizes.small
                RowLayout {
                    Layout.preferredWidth: mimeTypesView.columnSize
                    Layout.maximumWidth: mimeTypesView.columnSize
                    Layout.fillHeight: true
                    CheckBox {
                        Layout.fillHeight: true
                        checked: mimeTypesModel.checkedTypes.indexOf(delegate.name) >= 0
                        onToggled: {
                            let idx = mimeTypesModel.checkedTypes.indexOf(delegate.name);
                            if (idx >= 0) {
                                mimeTypesModel.checkedTypes.splice(idx, 1);
                            } else {
                                mimeTypesModel.checkedTypes.push(delegate.name)
                            }
                        }
                    }
                    Kirigami.Icon {
                        Layout.fillHeight: true
                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                        animated: false // TableView reuses delegates, avoid animation when sorting/filtering.
                        source: delegate.decoration
                    }
                    Label {
                        text: delegate.name
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
                Label {
                    text: delegate.comment
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }
    footer: RowLayout {
        id: selectLayout
        Button {
            id: selectAllButton
            enabled: (filterMode.currentIndex > 0)
            icon.name: "edit-select-all"
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.visible: (Kirigami.Settings.isMobile ? pressed : hovered) && ToolTip.text.length > 0
            ToolTip.text: i18nc("@action:button tooltip only Select all filetypes", "Select All")
            onClicked: filteredMimeTypesModel.checkFiltered()
        }

        Button {
            id: deselectAllButton
            enabled: (filterMode.currentIndex > 0)
            icon.name: "edit-select-none"
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.visible: (Kirigami.Settings.isMobile ? pressed : hovered) && ToolTip.text.length > 0
            ToolTip.text: i18nc("@action:button tooltip only Deselect all filetypes", "Deselect All")
            onClicked: filteredMimeTypesModel.uncheckFiltered()
        }

        Button {
            enabled: (filterMode.currentIndex > 0)
            icon.name: filteredMimeTypesModel.sortOrder === Qt.AscendingOrder ? "view-sort-ascending-symbolic" : "view-sort-descending-symbolic"
            ToolTip.delay: Kirigami.Units.toolTipDelay
            ToolTip.visible: (Kirigami.Settings.isMobile ? pressed : hovered) && ToolTip.text.length > 0
            ToolTip.text: i18nc("@action:button tooltip only, ascending/descending", "Switch Sort Order")
            onClicked: {
                filteredMimeTypesModel.sortOrder = filteredMimeTypesModel.sortOrder === Qt.AscendingOrder ? Qt.DescendingOrder : Qt.AscendingOrder;
                filteredMimeTypesModel.sort(0, filteredMimeTypesModel.sortOrder);
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }
}
