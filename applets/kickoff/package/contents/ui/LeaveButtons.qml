/*
    Copyright (C) 2020  Mikel Johnson <mikel5764@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 2.0
import org.kde.plasma.private.kicker 0.1 as Kicker
import org.kde.plasma.components 2.0 as PlasmaComponents // for Menu + MenuItem
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import QtQuick.Layouts 1.12

Item {
    id: leaveButtonRoot
    property Item leave: itemModel.contentWidth > itemModel.width ? leaveIconButton : leaveButton

    ListView {
        id: itemModelReference
        interactive: false
        visible: false
        // maximum available space (including leave button being icon only)
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: leaveIconButton.left
        anchors.rightMargin: PlasmaCore.Units.largeSpacing
        orientation: ListView.Horizontal
        spacing: PlasmaCore.Units.smallSpacing * 2
        delegate: PlasmaComponents3.ToolButton {
            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
            text: model.display
            icon.name: model.decoration
            // It's important to ignore here, as it will cause crashes when using orca in some cases (like when removing applet or restarting)
            Accessible.ignored: true
        }
        model: systemFavorites
        // Important
        Accessible.ignored: true
    }
    ListView {
        id: itemModel
        interactive: false
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: leaveButton.left
        anchors.rightMargin: PlasmaCore.Units.largeSpacing
        orientation: ListView.Horizontal
        spacing: PlasmaCore.Units.smallSpacing * 2
        delegate: PlasmaComponents3.ToolButton {
            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
            text: model.display
            icon.name: model.decoration
            onClicked: {
                itemModel.model.trigger(index, "", "")
            }
            //intentionally allow overflow (in that case leave button is icon only to account for this)
            width: itemModelReference.contentWidth <= itemModelReference.width ? implicitWidth : Math.min(implicitWidth, Math.round((itemModelReference.width-PlasmaCore.Units.smallSpacing * 2)/itemModel.count))
        }
        model: systemFavorites
        // don't let view move to last child (thus breaking stuff) by going to beginning on width change
        // lesson learned: don't do onContentXChanged because it will cause loops and *will* crash the desktop in RTL mode
        onWidthChanged: positionViewAtBeginning()
    }

    PlasmaComponents3.ToolButton {
        id: leaveButton
        property int currentID: plasmoid.configuration.primaryActions
        icon.name: currentID == 0 ? "system-log-out" :  currentID == 1 ? "system-shutdown" : "view-more-symbolic"
        onClicked: contextMenu.open()
        text: currentID == 0 ? i18n("Leave...") : currentID == 1 ? i18n("Power...") : i18n("More...")
        visible: itemModel.contentWidth <= itemModel.width
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter;
        Keys.forwardTo: [leaveButtonRoot]
    }
    PlasmaComponents3.ToolButton {
        id: leaveIconButton
        icon.name: leaveButton.icon.name
        onClicked: contextMenu.open()
        visible: itemModel.contentWidth > itemModel.width
        display: PlasmaComponents3.AbstractButton.IconOnly
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter;
        hoverEnabled: true
        PlasmaComponents3.ToolTip { text: leaveButton.currentID == 0 ? i18n("Leave") : leaveButton.currentID == 1 ? i18n("Power") : i18n("More") }
        Keys.forwardTo: [leaveButtonRoot]
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_Tab && mainTabGroup.state == "top") {
            keyboardNavigation.state = "LeftColumn"
            root.currentView.forceActiveFocus()
            event.accepted = true;
        }
    }

    ListView {
        id: itemMenuModel
        delegate: Item {
            property string name: display
            property string icon: decoration
            property string info: description
            property string actionId: favoriteId
        }
        model: Kicker.SystemModel { favoritesModel: globalFavorites }
    }
    PlasmaComponents.Menu {
        id: contextMenu
        visualParent: itemModel.contentWidth > itemModel.width ? leaveIconButton : leaveButton
        Component.onCompleted: {
            for (var i = 0; i < itemMenuModel.count; i++) {
                contextMenuItemComponent.createObject(contextMenu, { "actionItem" : itemMenuModel.itemAtIndex(i), "index": i });
            }
        }
    }
    Component {
        id: contextMenuItemComponent

        PlasmaComponents.MenuItem {
            property var actionItem
            property int index: -1
            text: actionItem && actionItem.name ? actionItem.name : ""
            icon: actionItem && actionItem.icon ? actionItem.icon : null
            visible: actionItem && !String(plasmoid.configuration.systemFavorites).includes(actionItem.actionId)

            onClicked: {
                if (index >= 0) {
                    itemMenuModel.model.trigger(index, "", "")
                }
            }
        }
    }

}
