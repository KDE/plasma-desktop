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
    id: root

    readonly property int itemIndex: model.index
    readonly property bool isDropPlaceHolder: "dropPlaceholderIndex" in root.baseModel && root.itemIndex === root.baseModel.dropPlaceholderIndex
    property bool showUnfavoritePlaceholder: false

    Accessible.role: Accessible.ListItem
    icon.source: root.decoration
    icon.width: Kirigami.Units.iconSizes.medium
    icon.height: Kirigami.Units.iconSizes.medium
    hoverEnabled: true
    dragActive: dragHandler.active

    background.visible: false // we want the default background's spacing, but not the base color
    contentItem: Kirigami.Icon {
        visible: !root.showUnfavoritePlaceholder && !root.isDropPlaceHolder
        active: root.hovered
        width: root.icon.width
        height: root.icon.height
        source: root.icon.source
    }

    Loader {
        active: root.hovered || root.visualFocus || dragHandler.active || root.isDropPlaceHolder || root.showUnfavoritePlaceholder
        anchors.fill: parent

        sourceComponent: Item {
            id: highlightItem

            anchors.fill: parent

            PlasmaExtras.Highlight {
                anchors.fill: parent
                visible: !root.isDropPlaceHolder && !root.showUnfavoritePlaceholder
                hovered: true
                pressed: root.pressed
            }

            KSvg.FrameSvgItem {
                anchors.fill: parent

                visible: root.isDropPlaceHolder || root.showUnfavoritePlaceholder

                imagePath: "widgets/viewitem"
                prefix: "selected"

                opacity: 0.5

                Kirigami.Icon {
                    anchors.centerIn: parent

                    width: root.icon.width
                    height: width

                    source: root.isDropPlaceHolder ? "list-add" : "list-remove"
                    active: false
                }
            }
        }
    }

    DragHandler {
        id: dragHandler
        target: null
        onActiveChanged: if (active) {
            root.contentItem.grabToImage(function(result) {
                root.Drag.imageSource = result.url
                root.Drag.active = true // using a binding can cause loop warnings when unexpanding
            })
        } else {
            root.Drag.active = false
        }
    }
    Drag.dragType: Drag.Automatic
    Drag.mimeData: {
        'favoritedrag': '',
        "text/uri-list" : [root.url]
    }

    PC3.ToolTip {
        text: root.text
    }
}
