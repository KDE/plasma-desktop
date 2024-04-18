/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels
import org.kde.kcmutils as KCM

ColumnLayout {
    spacing: Kirigami.Units.smallSpacing

    Connections {
        target: kcm.xkbOptionsModel

        function onNavigateTo(index): void {
            const modelIndex = xkbOptionsProxy.mapFromSource(index)
            treeView.expand(modelIndex.row)
            treeView.positionViewAtIndex(modelIndex, Qt.AlignCenter)
            treeView.forceActiveFocus()
        }
    }

    KItemModels.KSortFilterProxyModel {
        id: xkbOptionsProxy
        sourceModel: kcm?.xkbOptionsModel ?? null

        autoAcceptChildRows: true
        recursiveFilteringEnabled: true
        filterRoleName: "display"
        // filterCaseSensitivity: Qt.CaseInsensitive
        // filterString: searchField.text
        filterRegularExpression: {
            function escapeRegex(string) {
                https://stackoverflow.com/questions/3561493/is-there-a-regexp-escape-function-in-javascript
                return string.replace(/[/\-\\^$*+?.()|[\]{}]/g, '\\$&');
            }
            return new RegExp(searchField.text.split(/\s+/).map(escapeRegex).join(".*"), "i")
        }

        onCountChanged: {
            if (searchField.text.length > 0) {
                treeView.expandRecursively();
            } else {
                treeView.collapseRecursively();
            }
        }

        sortRoleName: "display"
        sortOrder: Qt.AscendingOrder
    }

    QQC2.CheckBox {
        text: i18nc("@option:checkbox", "Configure keyboard options")
        checked: kcm.keyboardSettings.resetOldXkbOptions
        onToggled: kcm.keyboardSettings.resetOldXkbOptions = checked

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "resetOldXkbOptions"
        }
    }

    Kirigami.SearchField {
        id: searchField
        Layout.fillWidth: true
    }

    QQC2.ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        Component.onCompleted: {
            if (background) {
                background.visible = true
            }
        }

        TreeView {
            id: treeView
            boundsBehavior: Flickable.StopAtBounds
            alternatingRows: false
            clip: true

            onEnabledChanged: {
                if (!enabled) {
                    for (let i in rows) {
                        collapse(i)
                    }
                }
            }

            columnWidthProvider: column => {
                switch (column) {
                case 0:
                    return width;
                default:
                    return 0;
                }
            }

            selectionModel: ItemSelectionModel {}
            keyNavigationEnabled: true

            model: xkbOptionsProxy

            delegate: QQC2.TreeViewDelegate {
                id: delegate

                required current

                activeFocusOnTab: false

                // you can't use a CheckDelegate as a TreeViewDelegate. This nests the buttons and is pretty bad for accessible
                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.CheckBox {
                        id: checkbox
                        activeFocusOnTab: false
                        tristate: delegate.isTreeNode && delegate.hasChildren
                        checkState: delegate.model.checkState
                        onToggled: delegate.model.checkState = checkState

                        KCM.SettingHighlighter {
                            highlight: delegate.model.checkState !== Qt.Unchecked
                        }
                    }

                    QQC2.Label {
                        Layout.fillWidth: true
                        text: delegate.model.display
                    }
                }

                onClicked: {
                    if (delegate.isTreeNode && delegate.hasChildren) {
                        treeView.toggleExpanded(row)
                    } else {
                        delegate.model.checkState = checkbox.checkState === Qt.Checked ? Qt.Unchecked : Qt.Checked
                    }
                }
            }
        }
    }
}
