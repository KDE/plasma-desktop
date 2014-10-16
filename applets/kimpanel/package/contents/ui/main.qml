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

    function action_reloadconfig() {
         action("ReloadConfig");
    }

    function action_configureim() {
         action("Configure");
    }

    function action_exit() {
         action("Exit");
    }

    Component.onCompleted: {
        plasmoid.setAction("configureim", i18n("Configure Input Method"), "configure");
        plasmoid.setAction("reloadconfig", i18n("Reload Config"), "view-refresh");
        plasmoid.setAction("exit", i18n("Exit Input Method"), "application-exit");
    }


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
                anchors.centerIn: parent
                width: items.iconSize
                height: items.iconSize
                label: model.label
                tip: model.tip
                icon: model.icon
                hint: model.hint
                onTriggered : clickHandler(model.key)
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

    Timer {
        id: timer
        interval: 50
        onTriggered: {
            var data = dataEngine.data["statusbar"]["Properties"];
            if (!data) {
                return;
            }
            kimpanel.visibleButtons = data.length;
            var count = list.count;
            if (data.length < count) {
                list.remove(data.length, count - data.length);
            }

            var i;
            for (i = 0; i < data.length; i ++) {
                var itemData = {'key': data[i].key,
                                'icon': data[i].icon,
                                'label': data[i].label,
                                'tip': data[i].tip,
                                'hint': data[i].hint };
                if (i < count) {
                    list.set(i, itemData);
                } else {
                    list.append(itemData);
                }
            }
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

