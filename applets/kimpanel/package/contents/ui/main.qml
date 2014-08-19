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

Flow {
    id: kimpanel
    property bool vertical: plasmoid.formFactor == PlasmaCore.Types.Vertical
    Layout.minimumWidth: vertical ? 0 : height * visibleButtons
    Layout.minimumHeight: vertical ? width * visibleButtons : 0
    Layout.preferredWidth: Layout.minimumWidth
    Layout.preferredHeight: Layout.minimumHeight

    property int minButtonSize: 16

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    property alias visibleButtons: items.count;

    flow: vertical ? Flow.TopToBottom : Flow.LeftToRight

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

    Repeater {
        id: items
        property int itemWidth: parent.flow==Flow.LeftToRight ? Math.floor(parent.width/visibleButtons) : parent.width
        property int itemHeight: parent.flow==Flow.TopToBottom ? Math.floor(parent.height/visibleButtons) : parent.height
        property int iconSize: Math.min(itemWidth, itemHeight)

        model: ListModel {
            id: list
            dynamicRoles: true
        }

        delegate: Item {
            id: iconDelegate
            width: items.itemWidth
            height: items.itemHeight
            StatusIcon {
                width: items.iconSize
                height: items.iconSize
                label: model.label
                tip: model.tip
                icon: model.icon
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
                actionList.push({"actionId": menuData["props"][i].key, "icon": menuData["props"][i].icon, "text": menuData["props"][i].label});
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

    PlasmaCore.DataSource {
        id: dataEngine
        engine: "kimpanel"
        connectedSources: ["statusbar"]
        onDataChanged: {
            showMenu(actionMenu, dataEngine.data["statusbar"]["Menu"]);

            var data = dataEngine.data["statusbar"]["Properties"];
            if (!data) {
                return;
            }

            var newKeys = {};
            var i, j;
            for (i = 0; i < data.length; i ++) {
                newKeys[data[i].key] = true;
            }
            i = 0;
            while (i < list.count) {
                if (newKeys[list.get(i).key]) {
                    i++;
                } else {
                    list.remove(i);
                }
            }

            for (i = 0; i < data.length; i ++) {
                var found = false;
                for (j = i; j < list.count; j++) {
                    if (list.get(j).key == data[i].key) {
                        found = true;
                        list.set(j, {'key': data[i].key,
                                     'icon': data[i].icon,
                                     'label': data[i].label,
                                     'tip': data[i].tip });
                        if (j != i) {
                            list.move(j, i, 1);
                        }
                        break;
                    }
                }
                if (!found) {
                    list.insert(i, {'key': data[i].key,
                                    'icon': data[i].icon,
                                    'label': data[i].label,
                                    'tip': data[i].tip });
                }
            }
        }
    }
}

