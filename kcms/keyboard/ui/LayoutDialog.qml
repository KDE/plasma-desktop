/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kquickcontrols as KQuickControls
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
        button.enabled = Qt.binding(() => layoutSelector.variantView.currentIndex >= 0);

        layoutSelector.forceSearchFocus();
    }

    onAccepted: {
        const layout = layoutSelector.currentVariant.shortName
        const variant = layoutSelector.currentVariant.variantName
        const shortcut = sequenceItem.keySequence;
        const displayName = layout === displayNameField.text ? "" : displayNameField.text

        kcm.userLayoutModel.addLayout(layout, variant, shortcut, displayName)
    }

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        LayoutSelector {
            id: layoutSelector
            Layout.fillWidth: true
            Layout.fillHeight: true
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
                    value: layoutSelector.currentVariant?.shortName ?? ""
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
        enabled: layoutSelector.variantView.currentIndex >= 0
        onClicked: kcm.requestPreview(
                       kcm.keyboardModel,
                       layoutSelector.currentVariant.shortName,
                       layoutSelector.currentVariant.variantName
                       )
    }
}
