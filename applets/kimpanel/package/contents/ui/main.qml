/*
 *  Copyright 2014 Weng Xuetian <wengxt@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: kimpanel
    property int visibleButtons: 0

    Layout.minimumWidth: vertical ? 0 : height * visibleButtons
    Layout.minimumHeight: vertical ? width * visibleButtons : 0
    Layout.preferredWidth: Layout.minimumWidth
    Layout.preferredHeight: Layout.minimumHeight

    property bool vertical: plasmoid.formFactor == PlasmaCore.Types.Vertical

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    InputPanel { }

    GridView {
        id: items
        property int itemWidth: !kimpanel.vertical ? Math.floor(parent.width/visibleButtons) : parent.width
        property int itemHeight: kimpanel.vertical ? Math.floor(parent.height/visibleButtons) : parent.height
        property int iconSize: Math.min(itemWidth, itemHeight)
        anchors.fill: parent
        cellWidth: itemWidth
        cellHeight: itemHeight
        interactive: false

        model: ListModel {
            id: list
            dynamicRoles: true
        }

        delegate: Item {
            id: iconDelegate
            width: items.itemWidth
            height: items.itemHeight
            StatusIcon {
                id: statusIcon
                anchors.centerIn: parent
                width: items.iconSize
                height: items.iconSize
                label: model.label
                tip: model.tip
                icon: model.icon
                hint: model.hint
                onTriggered : {
                    if (button == Qt.LeftButton) {
                        clickHandler(model.key)
                    } else {
                        contextMenu.open(statusIcon, {key: model.key, label: model.label});
                    }
                }
            }
        }
    }

    function clickHandler(key) {
        var service = dataEngine.serviceForSource("statusbar");
        var operation = service.operationDescription("TriggerProperty");
        operation.key = key;
        service.startOperationCall(operation);
    }

    function action(key) {
        var service = dataEngine.serviceForSource("statusbar");
        var operation = service.operationDescription(key);
        service.startOperationCall(operation);
    }

    function hideAction(key) {
        // We must use assignment to change the configuration property,
        // otherwise it won't get notified.
        var hiddenList = plasmoid.configuration.hiddenList;
        if (hiddenList.indexOf(key) === -1) {
            hiddenList.push(key);
            plasmoid.configuration.hiddenList = hiddenList;
        }
        timer.restart();
    }

    function showAction(key) {
        // We must use assignment to change the configuration property,
        // otherwise it won't get notified.
        var hiddenList = plasmoid.configuration.hiddenList;
        var index = hiddenList.indexOf(key);
        if (index !== -1) {
            hiddenList.splice(index, 1);
            plasmoid.configuration.hiddenList = hiddenList;
        }
        timer.restart();
    }

    function showMenu(menu, menuData) {
        if (!menuData) {
            return;
        }

        if (menuData["timestamp"] > menu.timestamp) {
            menu.timestamp = menuData["timestamp"];
            var actionList = [];
            for (var i = 0; i < menuData["props"].length; i++ ) {
                actionList.push({"actionId": menuData["props"][i].key, "icon": menuData["props"][i].icon, "text": menuData["props"][i].label, hint: menuData["props"][i].hint});
            }
            if (actionList.length > 0) {
                menu.actionList = actionList;
                menu.open();
            }
        }
    }

    ActionMenu {
        property var timestamp: 0;
        id: actionMenu
        onActionClicked: {
            clickHandler(actionId);
        }
    }

    ContextMenu {
        id: contextMenu
    }

    Timer {
        id: timer
        interval: 50
        onTriggered: {
            var data = dataEngine.data["statusbar"]["Properties"];
            if (!data) {
                return;
            }
            var count = list.count;
            var c = 0, i;
            var hiddenActions = [];
            for (i = 0; i < data.length; i ++) {
                if (plasmoid.configuration.hiddenList.indexOf(data[i].key) !== -1) {
                    hiddenActions.push({'key': data[i].key,
                                'icon': data[i].icon,
                                'label': data[i].label});
                } else {
                    c = c + 1;
                }
            }
            if (c < count) {
                list.remove(c, count - c);
            }
            kimpanel.visibleButtons = c;

            c = 0;
            for (i = 0; i < data.length; i ++) {
                if (plasmoid.configuration.hiddenList.indexOf(data[i].key) !== -1) {
                    continue;
                }
                var itemData = {'key': data[i].key,
                                'icon': data[i].icon,
                                'label': data[i].label,
                                'tip': data[i].tip,
                                'hint': data[i].hint };
                if (c < count) {
                    list.set(c, itemData);
                } else {
                    list.append(itemData);
                }
                c = c + 1;
            }
            contextMenu.actionList = hiddenActions;
        }
    }

    PlasmaCore.DataSource {
        id: dataEngine
        engine: "kimpanel"
        connectedSources: ["statusbar"]
        onDataChanged: {
            showMenu(actionMenu, dataEngine.data["statusbar"]["Menu"]);
            var data = dataEngine.data["statusbar"]["Properties"];
            if (!data) {
                kimpanel.visibleButtons = 0;
                return;
            }

            timer.restart();
        }
    }
}

