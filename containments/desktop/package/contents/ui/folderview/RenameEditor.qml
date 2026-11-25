/*
    SPDX-FileCopyrightText: 2026 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

PlasmaComponents.ScrollView {
    id: root

    // The item the editor is parented to and positioned in (the layout)
    required property Item view
    // The FolderModel, used to compute the file extension boundary
    required property var model

    property alias text: editor.text
    property Item targetItem: null

    signal commit

    visible: false

    onFocusChanged: {
        if (focus) {
            editor.forceActiveFocus();
        }
    }

    onTargetItemChanged: {
        if (targetItem !== null) {
            var xy = editor.getXY();
            root.x = xy[0];
            root.y = xy[1];
            root.width = editor.getWidth();
            root.height = editor.getInitHeight();
            text = targetItem.display;
            editor.adjustSize();
            editor.select(0, dir.fileExtensionBoundary(targetItem.index));
            if (root.inPanel) {
                root.contentItem.contentX = Math.max(root.contentItem.contentWidth - contentItem.width, 0);
            } else {
                root.contentItem.contentY = Math.max(root.contentItem.contentHeight - contentItem.height, 0);
            }
            root.visible = true;
        } else {
            root.x = 0;
            root.y = 0;
            root.visible = false;
        }
    }


    // We use QQC2.TextArea here to allow context menu to appear,
    // since PlasmaComponents.TextArea does not have a context menu
    // BUG:427292
    QQC2.TextArea {
        id: editor

        wrapMode: TextEdit.Wrap
        textMargin: 0
        horizontalAlignment: TextEdit.AlignHCenter

        Kirigami.SpellCheck.enabled: false

        // Temporary workaround until PlasmaComponents.TextArea gets a context menu
        color: Kirigami.Theme.textColor

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            radius: Kirigami.Units.cornerRadius
            border.color: Kirigami.Theme.highlightColor
            border.width: 1
        }


        Binding {
            target: editor.background
            property: "width"
            value: root.width
        }
        Binding {
            target: editor.background
            property: "height"
            value: root.height
        }
        Component.onCompleted: root.contentItem.clip = false

        Keys.onPressed: event => {
            switch (event.key) {
            case Qt.Key_Return:
            case Qt.Key_Enter:
                root.commit();
                break;
            case Qt.Key_Escape:
                if (targetItem) {
                    targetItem = null;
                    event.accepted = true;
                }
                break;
            case Qt.Key_Home:
                if (event.modifiers & Qt.ShiftModifier) {
                    editor.select(0, editor.cursorPosition);
                } else {
                    editor.select(0, 0);
                }
                event.accepted = true;
                break;
            case Qt.Key_End:
                if (event.modifiers & Qt.ShiftModifier) {
                    editor.select(editor.cursorPosition, editor.text.length);
                } else {
                    editor.select(editor.text.length, editor.text.length);
                }
                event.accepted = true;
                break;
            default:
                adjustSize();
                break;
            }
        }

        onTextChanged: adjustSize()

        function getXY() {
            if (!targetItem) {
                return [0, 0];
            }
            const pos = root.view.mapFromItem(targetItem, targetItem.labelArea.x, targetItem.labelArea.y);
            return [pos.x - editor.leftPadding, pos.y + Kirigami.Units.smallSpacing - editor.topPadding];
        }

        function getWidth(addWidthVerticalScroller) {
            if (!targetItem) {
                return 0;
            }
            return targetItem.labelArea.parent.width - Kirigami.Units.smallSpacing +
                (addWidthVerticalScroller ? root.PlasmaComponents.ScrollBar.vertical.width : 0);
        }

        function getHeight(addWidthHorizontalScroller, init) {
            if (!targetItem) {
                return 0;
            }
            let height;
            if (init) {
                height = targetItem.labelArea.height + editor.topPadding + editor.bottomPadding;
            } else {
                const realHeight = contentHeight + editor.topPadding + editor.bottomPadding;
                const maxHeight = Kirigami.Units.iconSizes.sizeForLabels * (Plasmoid.configuration.textLines + 1) + editor.topPadding + editor.bottomPadding;
                height = Math.min(realHeight, maxHeight);
            }
            return height + (addWidthHorizontalScroller ? root.PlasmaComponents.ScrollBar.horizontal.height : 0);
        }

        function getInitHeight() {
            return getHeight(false, true);
        }

        function adjustSize() {
            root.height = getHeight();
            if (contentHeight + editor.topPadding + editor.bottomPadding > root.height) {
                root.visible = targetItem !== null;
                root.PlasmaComponents.ScrollBar.vertical.policy = Qt.ScrollBarAlwaysOn;
                root.width = getWidth(true);
            } else {
                root.PlasmaComponents.ScrollBar.vertical.policy = Qt.ScrollBarAlwaysOff;
                root.width = getWidth();
            }

            const xy = getXY();
            root.x = xy[0];
            root.y = xy[1];
        }
    }
}
