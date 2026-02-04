/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PC3
import org.kde.plasma.extras as PlasmaExtras

ItemAbstractDelegate {
    id: item

    readonly property int itemIndex: model.index

    Accessible.role: Accessible.ListItem
    icon.source: item.decoration
    icon.width: Kirigami.Units.iconSizes.medium
    icon.height: Kirigami.Units.iconSizes.medium
    hoverEnabled: true

    Keys.onSpacePressed: clicked()

    background.visible: false // we want the default background's spacing, but not the base color
    contentItem: Kirigami.Icon {
        active: item.hovered
        width: item.icon.width
        height: item.icon.height
        source: item.icon.source
    }

    PlasmaExtras.Highlight {
        anchors.fill: parent
        hovered: item.hovered || item.visualFocus || dragHandler.active
        pressed: tapHandler.pressed
    }

    TapHandler {
        // dedicated tapHandler as ItemDelegate's clicked conflicts with DragHandler
        id: tapHandler
        onTapped: {
            favoritesModel.trigger(index, "", null);
            interactionConcluded()
        }
    }

    DragHandler {
        id: dragHandler
        target: null
        onActiveChanged: if (active) {
            item.contentItem.grabToImage(function(result) {
                item.Drag.imageSource = result.url
                item.Drag.active = true // using a binding can cause loop warnings when unexpanding
            })
        } else {
            item.Drag.active = false
        }
    }
    Drag.dragType: Drag.Automatic
    Drag.mimeData: {
        "text/uri-list" : [item.url]
    }

    PC3.ToolTip {
        text: item.text
    }
}
