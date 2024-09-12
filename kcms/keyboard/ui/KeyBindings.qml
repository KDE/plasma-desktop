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

import Qt.labs.qmlmodels

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
        recursiveFilteringEnabled: true
        autoAcceptChildRows: true
        filterRoleName: "display"

        filterRegularExpression: {
            function escapeRegex(string) {
                // https://stackoverflow.com/questions/3561493/is-there-a-regexp-escape-function-in-javascript
                return string.replace(/[/\-\\^$*+?.()|[\]{}]/g, '\\$&');
            }

            return new RegExp(searchField.text.split(/\s+/).map(escapeRegex).join(".*"), "i");
        }

        onCountChanged: {
            if (searchField.text.length > 0) {
                treeView.expandRecursively();
            } else {
                treeView.collapseRecursively();
            }
        }
    }

    header: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.CheckBox {
            text: i18nc("@label:checkbox", "Configure keyboard options")
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
    }

    view: TreeView {
        id: treeView
        activeFocusOnTab: true
        alternatingRows: false
        clip: true
        enabled: kcm.keyboardSettings.resetOldXkbOptions
        reuseItems: false

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
        delegate: chooser
    }

    DelegateChooser {
        id: chooser
        role: "type"

        DelegateChoice {
            roleValue: "parentNode"

            QQC2.TreeViewDelegate {
                activeFocusOnTab: true
                onClicked: treeView.toggleExpanded(row)

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Label {
                        text: model.display
                        Layout.fillWidth: true
                        font.bold: model.checked
                    }

                    QQC2.ToolButton {
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        icon.name: "edit-reset-symbolic"
                        text: i18nc("@action:button", "Reset")
                        visible: model.checked
                        onClicked: kcm.xkbOptionsModel.clearXkbGroup(model.name)

                        Accessible.description: i18nc("@info accessible", "Reset selected options for this group")
                        QQC2.ToolTip.text: i18nc("@info:tooltip", "Reset selected options for this group")
                        QQC2.ToolTip.visible: hovered
                    }
                }
            }
        }

        DelegateChoice {
            roleValue: "radio"

            QQC2.TreeViewDelegate {
                activeFocusOnTab: false
                contentItem: QQC2.RadioButton {
                    activeFocusOnTab: true
                    checked: model.checked
                    onToggled: model.checked = !model.checked
                    text: model.display
                    // Checked state controlled via xkbOptionsModel
                    autoExclusive: false

                    KCM.SettingHighlighter {
                        highlight: model.checked
                    }
                }

                onClicked: model.checked = !model.checked
            }
        }


        DelegateChoice {
            roleValue: "check"

            QQC2.TreeViewDelegate {
                activeFocusOnTab: false
                contentItem: QQC2.CheckBox {
                    activeFocusOnTab: true
                    checked: model.checked
                    onToggled: model.checked = !model.checked
                    text: model.display

                    KCM.SettingHighlighter {
                        highlight: model.checked
                    }
                }

                onClicked: model.checked = !model.checked
            }
        }
    }
}
