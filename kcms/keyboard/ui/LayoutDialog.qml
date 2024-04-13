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
import org.kde.kcmutils as KCM

import org.kde.plasma.private.kcm_keyboard as KCMKeyboard

Kirigami.Dialog {
    id: dialog

    title: i18nc("@title:window", "Add Layout")
    modal: true

    implicitWidth: Kirigami.Units.gridUnit * 32
    implicitHeight: Kirigami.Units.gridUnit * 24

    padding: Kirigami.Units.largeSpacing

    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel
    Component.onCompleted: {
        let button = standardButton(QQC2.Dialog.Ok);
        button.enabled = Qt.binding(() => layoutsView.currentIndex >= 0);
    }

    onAccepted: {
        const layout = layoutsView.currentItem.shortName
        const variant = layoutsView.currentItem.variantName
        const shortcut = sequenceItem.keySequence;
        const displayName = layout === displayNameField.text ? "" : displayNameField.text

        kcm.userLayoutModel.addLayout(layout, variant, shortcut, displayName)
    }

    KItemModels.KSortFilterProxyModel {
        id: layoutsProxy
        sourceModel: kcm?.layouts ?? undefined

        filterRoleName: "description"
        sortRoleName: "description"
        sortOrder: Qt.AscendingOrder
    }

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.SearchField {
            Layout.fillWidth: true
            // Likewise explicitly setting filterRegularExpression changes the current case sensitivity, thereby breaking its binding
            onAccepted: layoutsProxy.filterRegularExpression = new RegExp(text, "i")
        }

        QQC2.ScrollView {
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true

            Component.onCompleted: {
                if (background) {
                    background.visible = true;
                }
            }

            ListView {
                id: layoutsView

                model: layoutsProxy
                boundsBehavior: Flickable.StopAtBounds
                currentIndex: -1

                delegate: QQC2.ItemDelegate {
                    id: delegate
                    width: layoutsView.width
                    highlighted: ListView.isCurrentItem
                    onClicked: layoutsView.currentIndex = index

                    required property string shortName
                    required property string description
                    required property string variantName
                    required property int index

                    contentItem: RowLayout {
                        spacing: Kirigami.Units.smallSpacing

                        Kirigami.Icon {
                            source: KCMKeyboard.Flags.getIcon(delegate.shortName)
                            visible: valid
                        }

                        QQC2.Label {
                            Layout.fillWidth: true
                            text: delegate.description
                        }

                        QQC2.ToolButton {
                            icon.name: "input-keyboard-virtual-symbolic"
                            text: i18nc("@action:button", "Preview")
                            display: QQC2.ToolButton.IconOnly
                            visible: delegate.hovered

                            QQC2.ToolTip.text: text
                            QQC2.ToolTip.visible: hovered

                            onClicked: kcm.requestPreview(kcm.keyboardSettings.keyboardModel, delegate.shortName, delegate.variantName)
                        }
                    }
                }
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.Label {
                text: i18nc("@label:textbox", "Display text:")
            }

            QQC2.TextField {
                id: displayNameField
                maximumLength: 3

                Binding {
                    target: displayNameField
                    property: "text"
                    value: layoutsView.currentItem?.shortName ?? ""
                }
            }

            Item {
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: i18nc("@option:textbox", "Shortcut:")
            }

            KQuickControls.KeySequenceItem {
                id: sequenceItem
                // To avoid fillHeight from clear button
                Layout.fillHeight: false
            }
        }
    }
}
