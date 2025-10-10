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

    property bool isMobile: false

    title: isMobile ? i18nc("@title:window", "Set Layout") : i18nc("@title:window", "Add Layout")

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

        if (!isMobile) {
            kcm.userLayoutModel.addLayout(layout, variant, shortcut, displayName)
        } else {
            kcm.userLayoutModel.setSingleLayout(layout, variant, shortcut, displayName)
        }
    }

    KCMKeyboard.LayoutSearchModel {
        id: layoutSearchProxy
        sourceModel: KCMKeyboard.LayoutModel {}
        searchString: ""
    }

    KeyboardLayoutsModel {
        id: layoutsProxy
    }

    KeyboardVariantsModel {
        id: variantProxy
    }

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.SearchField {
            id: searchField
            Layout.fillWidth: true
            onAccepted: {
                layoutSearchProxy.searchString = searchField.text.trim();
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
            visible: !isMobile

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
