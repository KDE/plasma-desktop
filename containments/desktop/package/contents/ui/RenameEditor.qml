/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQml 2.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kirigami 2.20 as Kirigami


PlasmaComponents.ScrollView {
    id: root

    property alias text: editor.text
    property alias targetItem: editor.targetItem
    signal commit

    onFocusChanged: {
        if (focus) {
            editor.forceActiveFocus();
        }
    }

    PlasmaComponents.TextArea {
        id: editor

        wrapMode: root.useListViewMode ? TextEdit.NoWrap : TextEdit.Wrap

        textMargin: 0

        horizontalAlignment: root.useListViewMode ? TextEdit.AlignLeft : TextEdit.AlignHCenter

        rightPadding: root.PlasmaComponents.ScrollBar.vertical.visible ? root.PlasmaComponents.ScrollBar.vertical.width : 0

        property Item targetItem: null

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

        onTargetItemChanged: {
            if (targetItem !== null) {
                var xy = getXY();
                root.x = xy[0];
                root.y = xy[1];
                root.width = getWidth();
                root.height = getInitHeight();
                text = targetItem.name;
                adjustSize();
                editor.select(0, dir.fileExtensionBoundary(positioner.map(targetItem.index)));
                if (isPopup) {
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
                    editor.select(0, cursorPosition);
                } else {
                    editor.select(0, 0);
                }
                event.accepted = true;
                break;
            case Qt.Key_End:
                if (event.modifiers & Qt.ShiftModifier) {
                    editor.select(cursorPosition, text.length);
                } else {
                    editor.select(text.length, text.length);
                }
                event.accepted = true;
                break;
            default:
                adjustSize();
                break;
            }
        }

        Keys.onReleased: event => {
            adjustSize();
        }

        function getXY() {
            if (!targetItem) {
                return [0,0];
            }
            var pos = main.mapFromItem(targetItem, targetItem.labelArea.x, targetItem.labelArea.y);
            var _x, _y;
            if (root.useListViewMode) {
                _x = targetItem.labelArea.x - editor.leftPadding;
                _y = pos.y - editor.topPadding;
            } else {
                _x = targetItem.x + Math.abs(Math.min(gridView.contentX, gridView.originX));
                _x += editor.leftPadding;
                _x += scrollArea.viewport.x;
                if (root.PlasmaComponents.ScrollBar.vertical.policy === Qt.ScrollBarAlwaysOn
                    && gridView.effectiveLayoutDirection === Qt.RightToLeft) {
                    _x -= root.PlasmaComponents.ScrollBar.vertical.width;
                }
                _y = pos.y + Kirigami.Units.smallSpacing - editor.topPadding;
            }
            return [ _x, _y ];
        }

        function getWidth(addWidthVerticalScroller) {
            if (!targetItem) {
                return 0;
            }
            return(targetItem.label.parent.width - Kirigami.Units.smallSpacing +
                (root.useListViewMode ? -(editor.leftPadding + editor.rightPadding + Kirigami.Units.smallSpacing) : 0) +
                (addWidthVerticalScroller ? root.PlasmaComponents.ScrollBar.vertical.width : 0));
        }

        function getHeight(addWidthHoriozontalScroller, init) {
            if (!targetItem) {
                return 0;
            }
            var _height;
            if (isPopup || init) {
                _height = targetItem.labelArea.height + editor.topPadding + editor.bottomPadding;
            } else {
                var realHeight = contentHeight + editor.topPadding + editor.bottomPadding;
                var maxHeight = Kirigami.Units.iconSizes.sizeForLabels * (Plasmoid.configuration.textLines + 1) + editor.topPadding + editor.bottomPadding;
                _height = Math.min(realHeight, maxHeight);
            }
            return _height + (addWidthHoriozontalScroller ? root.PlasmaComponents.ScrollBar.horizontal.height : 0);
        }

        function getInitHeight() {
            return getHeight(false, true);
        }

        function adjustSize() {
            if (isPopup) {
                if(contentWidth + editor.leftPadding + editor.rightPadding > root.width) {
                    root.visible = targetItem !== null;
                    root.PlasmaComponents.ScrollBar.horizontal.policy = Qt.ScrollBarAlwaysOn;
                    root.height = getHeight(true);
                } else {
                    root.PlasmaComponents.ScrollBar.horizontal.policy = Qt.ScrollBarAlwaysOff;
                    root.height = getHeight();
                }
            } else {
                root.height = getHeight();
                if(contentHeight + editor.topPadding + editor.bottomPadding > root.height) {
                    root.visible = targetItem !== null;
                    root.PlasmaComponents.ScrollBar.vertical.policy = Qt.ScrollBarAlwaysOn;
                    root.width = getWidth(true);
                } else {
                    root.PlasmaComponents.ScrollBar.vertical.policy = Qt.ScrollBarAlwaysOff;
                    root.width = getWidth();
                }
            }

            var xy = getXY();
            root.x = xy[0];
            root.y = xy[1];
        }
    }
}

