/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Templates as T

import org.kde.kquickcontrolsaddons
import org.kde.kirigami 2 as Kirigami
import org.kde.plasma.plasmoid 2.0

import "code/tools.js" as Tools

T.Button {
    id: item

    background: null
    display: T.AbstractButton.IconOnly
    flat: true
    hoverEnabled: true
    text: model.display

    property bool hasActionList: ((model.favoriteId !== null)
        || (("hasActionList" in model) && (model.hasActionList !== null)))
    property int itemIndex: model.index

    QQC2.ToolTip.text: item.text
    QQC2.ToolTip.visible: item.text.length > 0 && contentItem.active
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    Keys.onMenuPressed: event => {
        if (menuHandler.enabled) {
            item.openActionMenu(item, item.width, item.height);
        } else {
            event.accepted = false;
        }
    }
    Keys.onReturnPressed: clicked()
    Keys.onEnterPressed: clicked()
    Keys.onDeletePressed: {
        repeater.itemAt(Math.max(model.index - 1, 0))?.forceActiveFocus(Qt.TabFocusReason);
        repeater.model.removeFavorite(model.favoriteId);
    }

    onClicked: {
        repeater.model.trigger(index, "", null);
        kicker.expanded = false;
    }

    function openActionMenu(visualParent, x = 0, y = 0) {
        const actionList = (model.hasActionList !== null) ? model.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, repeater.model, model.favoriteId);
        actionMenu.visualParent = visualParent;
        actionMenu.open(x, y);
    }

    Shortcut {
        enabled: item.activeFocus && model.index > 0
        sequence: "Ctrl+Shift+Up"
        onActivated: repeater.model.moveRow(model.index, model.index - 1)
    }

    Shortcut {
        enabled: item.activeFocus && model.index < repeater.count - 1
        sequence: "Ctrl+Shift+Down"
        onActivated: repeater.model.moveRow(model.index, model.index + 1)
    }

    ActionMenu {
        id: actionMenu

        onActionClicked: (actionId, actionArgument) => {
            if (Tools.triggerAction(repeater.model, model.index, actionId, actionArgument) === true) {
                kicker.expanded = false;
            }
        }
    }

    TapHandler {
        id: menuHandler
        enabled: item.hasActionList
        acceptedButtons: Qt.RightButton
        gesturePolicy: TapHandler.WithinBounds // Release grab when menu appears
        onPressedChanged: if (pressed) {
            item.openActionMenu(item, point.position.x, point.position.y);
        }
    }

    contentItem: Kirigami.Icon {
        id: icon

        active: item.hovered || item.activeFocus
        source: model.decoration

        // https://bugreports.qt.io/browse/QTBUG-63395
        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: item.clicked()
        }

        DragHandler {
            id: dragHandler
            onActiveChanged: if (active) {
                icon.grabToImage((result) => {
                    if (!dragHandler.active) {
                        return;
                    }
                    dragSource.Drag.imageSource = result.url;
                    dragSource.Drag.mimeData = {
                        "text/uri-list": model.url.toString(),
                    };
                    dragSource.sourceItem = item;
                    dragSource.Drag.active = dragHandler.active;
                });
            } else {
                dragSource.Drag.active = false;
            }
        }
    }
}
