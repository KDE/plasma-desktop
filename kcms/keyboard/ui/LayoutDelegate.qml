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

Item {
    id: itemDelegate

    implicitWidth: ListView.view.width
    implicitHeight: delegate.height

    property list<Kirigami.Action> actions
    property bool isLoopingLayout

    required property var model
    required property int index

    required property string layoutName
    required property string layout
    required property string variant
    required property string variantName
    required property string shortcut
    required property string displayName

    readonly property var view: ListView.view
    readonly property int keySequenceItemWidth : keySequenceItem.implicitWidth

    signal move(int oldIndex, int newIndex)

    // Make grayed bg for looping layouts
    Rectangle {
        anchors.fill: parent
        Kirigami.Theme.colorSet: Kirigami.Theme.Window
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.alternateBackgroundColor
        visible: isLoopingLayout
    }

    QQC2.ItemDelegate {
        id: delegate
        implicitWidth: itemDelegate.width

        // There's no need for a list item to ever be selected
        down: false
        highlighted: false

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Kirigami.ListItemDragHandle {
                listItem: delegate
                listView: itemDelegate.view
                onMoveRequested: (oldIndex, newIndex) => itemDelegate.move(oldIndex, newIndex)
                visible: itemDelegate.view.count > 1
            }

            Kirigami.IconTitleSubtitle {
                icon.width: Kirigami.Units.iconSizes.smallMedium
                icon.height: Kirigami.Units.iconSizes.smallMedium
                icon.source: KCMKeyboard.Flags.getIcon(itemDelegate.layout)
                title: itemDelegate.layoutName
                subtitle: itemDelegate.variantName
                Layout.fillWidth: true
            }

            QQC2.TextField {
                Layout.preferredWidth: Kirigami.Units.gridUnit * 3
                maximumLength: 3
                horizontalAlignment: Text.AlignHCenter
                placeholderText: itemDelegate.layout
                background.visible: activeFocus

                text: itemDelegate.displayName
                onTextEdited: {
                    // Immediate response except for "", which resets to default
                    if (text.length > 0) {
                        itemDelegate.model.displayName = text
                    }
                }
                onEditingFinished: itemDelegate.model.displayName = text
                Keys.onEscapePressed: editingFinished()
            }

            // Spacer to align items to the left of the KeySequenceItem
            Item {
                Layout.preferredWidth: itemDelegate.view.maxKeySequenceItemWidth - keySequenceItem.implicitWidth
            }

            KQuickControls.KeySequenceItem {
                id: keySequenceItem
                showCancelButton: true
                patterns: KQuickControls.ShortcutPattern.Modifier | KQuickControls.ShortcutPattern.ModifierAndKey
                keySequence: itemDelegate.shortcut
                onKeySequenceChanged: itemDelegate.view.maxKeySequenceItemWidthChanged()
                onCaptureFinished: itemDelegate.model.shortcut = keySequence
            }

            Kirigami.ActionToolBar {
                Layout.fillWidth: false
                actions: itemDelegate.actions
            }
        }
    }
}
