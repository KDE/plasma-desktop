/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kquickcontrols as KQuickControls
import org.kde.kitemmodels as KItemModels
import org.kde.kirigami as Kirigami

import org.kde.plasma.private.kcm_keyboard as KCMKeyboard

Kirigami.Dialog {
    id: dialog

    title: i18nc("@title:window", "Add Layout")

    implicitWidth: Kirigami.Units.gridUnit * 34
    implicitHeight: Kirigami.Units.gridUnit * 26

    padding: Kirigami.Units.largeSpacing

    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel
    Component.onCompleted: {
        let button = standardButton(QQC2.Dialog.Ok);
        button.enabled = Qt.binding(() => variantView.currentIndex >= 0);

        searchField.forceActiveFocus();
    }

    onAccepted: {
        const layout = variantView.currentItem.shortName
        const variant = variantView.currentItem.variantName
        const shortcut = sequenceItem.keySequence;
        const displayName = layout === displayNameField.text ? "" : displayNameField.text

        kcm.userLayoutModel.addLayout(layout, variant, shortcut, displayName)
    }

    KItemModels.KSortFilterProxyModel {
        id: layoutsProxy
        sourceModel: kcm?.layouts ?? undefined
        sortRoleName: "description"
        sortOrder: Qt.AscendingOrder

        filterRowCallback: function (row, parent) {
            const modelIndex = sourceModel.index(row, 0, parent);
            const currentVariantName = sourceModel.data(modelIndex, KItemModels.KRoleNames.role("variantName"));

            if (currentVariantName !== '') {
                return false;
            }

            if (searchField.text.length > 0) {
                const currentDescription = sourceModel.data(modelIndex);

                if (currentDescription.search(filterRegularExpression) < 0) {
                    const shortNameRole = KItemModels.KRoleNames.role("shortName")
                    const currentName = sourceModel.data(modelIndex, shortNameRole);

                    for (let i = 0; i < sourceModel.rowCount(); i++) {
                        const index = sourceModel.index(i, 0, parent);
                        const name = sourceModel.data(index, shortNameRole);
                        const description = sourceModel.data(index);

                        if (name === currentName && description.search(filterRegularExpression) > -1) {
                            return true;
                        }
                    }

                    return false;
                }
            }

            return true;
        }
    }

    KItemModels.KSortFilterProxyModel {
        id: variantProxy
        sourceModel: kcm?.layouts ?? undefined

        filterRowCallback: function (row, parent) {
            if (!layoutsView.currentItem) {
                return false;
            }

            const modelIndex = sourceModel.index(row, 0, parent);
            const currentName = sourceModel.data(modelIndex, KItemModels.KRoleNames.role("shortName"));
            const selectedName = layoutsView.currentItem.shortName;

            if (currentName !== selectedName) {
                return false;
            }

            if (searchField.text.length > 0) {
                const currentDescription = sourceModel.data(modelIndex);
                if (currentDescription.search(filterRegularExpression) < 0) {
                    return false;
                }
            }

            return true;
        }
    }

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.SearchField {
            id: searchField
            Layout.fillWidth: true
            onAccepted: {
                const regexp = new RegExp(searchField.text.trim(), 'i');
                layoutsProxy.filterRegularExpression = regexp;
                variantProxy.filterRegularExpression = regexp;
            }
        }

        RowLayout {
            QQC2.ScrollView {
                clip: true
                implicitWidth: Math.round(dialog.width / 2)
                Layout.fillWidth: true
                Layout.fillHeight: true

                Component.onCompleted: {
                    if (background) {
                        background.visible = true
                    }
                }

                contentItem: ListView {
                    id: layoutsView
                    currentIndex: 0
                    model: layoutsProxy
                    delegate: LayoutDelegate {}
                    keyNavigationEnabled: true
                    activeFocusOnTab: true

                    onCurrentItemChanged: variantProxy.invalidateFilter()
                }
            }

            QQC2.ScrollView {
                clip: true
                implicitWidth: Math.round(dialog.width / 2)
                Layout.fillWidth: true
                Layout.fillHeight: true

                Component.onCompleted: {
                    if (background) {
                        background.visible = true
                    }
                }

                contentItem: ListView {
                    id: variantView
                    model: layoutsView.currentItem ? variantProxy : []
                    delegate: LayoutDelegate {}
                    keyNavigationEnabled: true
                    activeFocusOnTab: true
                }
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.TextField {
                id: displayNameField
                maximumLength: 3
                placeholderText: i18nc("@label:textbox", "Short layout name")

                Binding {
                    target: displayNameField
                    property: "text"
                    value: variantView.currentItem?.shortName ?? ""
                }
            }

            Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info", "The abbreviated name of the selected layout, which will be displayed in the system tray. You can change this setting yourself.")
            }

            Item {
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: i18nc("@option:textbox", "Shortcut:")
            }

            KQuickControls.KeySequenceItem {
                id: sequenceItem
            }
        }
    }

    footerLeadingComponent: QQC2.Button {
        icon.name: "input-keyboard-virtual-symbolic"
        text: i18nc("@action:button", "Preview")
        enabled: variantView.currentIndex >= 0
        onClicked: kcm.requestPreview(kcm.keyboardModel, variantView.currentItem.shortName, variantView.currentItem.variantName)
    }

    component LayoutDelegate: QQC2.ItemDelegate {
        id: delegate

        width: ListView.view.width
        highlighted: ListView.isCurrentItem

        required property string shortName
        required property string description
        required property string variantName
        required property int index

        onClicked: ListView.view.currentIndex = index

        Accessible.name: delegate.description

        contentItem: Kirigami.IconTitleSubtitle {
            icon.source: KCMKeyboard.Flags.getIcon(delegate.shortName)
            title: delegate.description

            QQC2.ToolTip.text: title
            QQC2.ToolTip.visible: delegate.hovered && truncated
        }
    }
}
