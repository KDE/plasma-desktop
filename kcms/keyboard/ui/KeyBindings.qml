/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    title: i18nc("@title", "Key Bindings")

    Connections {
        target: kcm.xkbOptionsModel

        function onNavigateTo(index): void {
            let modelIndex = xkbOptionsProxy.mapFromSource(index)
            treeView.expand(modelIndex.row)
            treeView.positionViewAtIndex(modelIndex, Qt.AlignCenter)
            treeView.forceActiveFocus()
        }
    }

    KItemModels.KSortFilterProxyModel {
        id: xkbOptionsProxy
        sourceModel: kcm?.xkbOptionsModel ?? undefined
        sortRoleName: "display"
        sortOrder: Qt.AscendingOrder
    }

    header: QQC2.CheckBox {
        text: i18nc("@label:checkbox", "Configure keyboard options")
        checked: kcm.keyboardSettings.resetOldXkbOptions
        onToggled: kcm.keyboardSettings.resetOldXkbOptions = checked

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "resetOldXkbOptions"
        }
    }

    view: TreeView {
        id: treeView
        boundsBehavior: Flickable.StopAtBounds
        alternatingRows: false
        clip: true
        enabled: kcm.keyboardSettings.resetOldXkbOptions

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

        model: xkbOptionsProxy

        delegate: QQC2.TreeViewDelegate {
            id: delegate

            // you can't use a CheckDelegate as a TreeViewDelegate. This nests the buttons and is pretty bad for accessible
            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                QQC2.CheckBox {
                    id: checkbox
                    tristate: delegate.isTreeNode && delegate.hasChildren
                    checkState: model.checkState
                    onToggled: model.checkState = checkState

                    KCM.SettingHighlighter {
                        highlight: model.checkState !== Qt.Unchecked
                    }
                }

                QQC2.Label {
                    Layout.fillWidth: true
                    text: model.display
                }
            }

            onClicked: {
                if (delegate.isTreeNode && delegate.hasChildren) {
                    treeView.toggleExpanded(row)
                } else {
                    model.checkState = checkbox.checkState === Qt.Checked ? Qt.Unchecked : Qt.Checked
                }
            }
        }
    }
}
