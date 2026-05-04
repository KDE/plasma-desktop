/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PC3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.ksvg as KSvg

ItemAbstractDelegate {
    id: item

    readonly property int itemIndex: model.index
    readonly property bool isDropPlaceHolder: "dropPlaceholderIndex" in item.baseModel && item.itemIndex === item.baseModel.dropPlaceholderIndex
    property bool showUnfavoritePlaceholder: false

    Accessible.role: Accessible.ListItem
    icon.source: item.decoration
    icon.width: Kirigami.Units.iconSizes.medium
    icon.height: Kirigami.Units.iconSizes.medium
    hoverEnabled: true
    dragActive: dragHandler.active

    Keys.onSpacePressed: action.trigger()

    background.visible: false // we want the default background's spacing, but not the base color
    contentItem: Kirigami.Icon {
        visible: !item.showUnfavoritePlaceholder
        active: item.hovered
        width: item.icon.width
        height: item.icon.height
        source: item.icon.source
    }

    Loader {
        active: item.hovered || item.visualFocus || dragHandler.active || item.isDropPlaceHolder || item.showUnfavoritePlaceholder
        anchors.fill: parent

        sourceComponent: Item {
            id: highlightItem

            anchors.fill: parent

            PlasmaExtras.Highlight {
                anchors.fill: parent
                visible: !item.isDropPlaceHolder && !item.showUnfavoritePlaceholder
                hovered: true
                pressed: item.pressed
            }

            KSvg.FrameSvgItem {
                anchors.fill: parent

                visible: item.isDropPlaceHolder || item.showUnfavoritePlaceholder

                imagePath: "widgets/viewitem"
                prefix: "selected"

                opacity: 0.5

                Kirigami.Icon {
                    anchors.centerIn: parent

                    width: item.icon.width
                    height: width

                    source: item.isDropPlaceHolder ? "list-add" : "list-remove"
                    active: false
                }
            }
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
        'favoritedrag': '',
        "text/uri-list" : [item.url]
    }

    PC3.ToolTip {
        text: item.text
    }
}
