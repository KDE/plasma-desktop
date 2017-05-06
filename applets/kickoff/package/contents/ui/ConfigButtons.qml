/*
 *  Copyright 2016 John Salatas <jsalatas@gmail.com>
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
import QtQuick.Layouts 1.0 as Layouts
import QtQuick.Controls 1.0 as Controls
import QtQml.Models 2.1
import org.kde.plasma.core 2.0 as PlasmaCore


GridView {
    id: configButtons

    cellHeight: units.gridUnit * 5
    cellWidth: units.gridUnit * 6

    width: cellWidth * 5
    height: cellHeight * 2

    property var items: {
        "bookmark":    { icon: "bookmarks", text: i18n("Favorites")},
        "application": { icon: "applications-other", text: i18n("Applications")},
        "computer":    { icon: "pm", text: i18n("Computer")},
        "used":        { icon: "view-history", text: i18n("History")},
        "oftenUsed":   { icon: "office-chart-pie", text: i18n("Often Used")},
        "leave":       { icon: "system-log-out", text: i18n("Leave")}
    }


    property var menuItems

    property var previousCell: [-1, -1]

    property alias listModel: visualModel.model
    property int sourceIndex: -1

    Component.onCompleted: {
        menuItems = plasmoid.configuration.menuItems;
        resetModel();
    }

    function updateModel() {
        var enabledItems = [];
        var disabledItems = [];
        for(var i = 0; i < menuItems.length; i++) {
            var confItemName = menuItems[i].substring(0, menuItems[i].indexOf(":"));
            var confItemEnabled = menuItems[i].substring(menuItems[i].length-1) == "t";

            var listItem = items[confItemName];
            listItem['name'] = confItemName;
            listItem['enabled'] = confItemEnabled;
            if(confItemEnabled) {
                enabledItems.push(listItem);
            } else {
                disabledItems.push(listItem);
            }
        }
        fillEmpty(enabledItems);
        fillEmpty(disabledItems);

        return enabledItems.concat(disabledItems);
    }

    function fillEmpty(list) {
        var emptyItem = { icon: undefined, text: undefined, name: 'empty', enabled: undefined};
        var itemsToAdd = 5 - list.length;
        for(var j = 0; j < itemsToAdd; j++) {
            list.push(emptyItem);
        }
    }

    function updateConfiguration() {
        menuItems = [];
        for(var i = 0; i < visualModel.items.count; i++) {
            var itemName = visualModel.items.get(i).model['name'];

            if(itemName != 'empty') {
                var configItem = itemName +":" +(i<5?"t":"f");
                menuItems.push(configItem);
            }

        }
        resetModel();
    }

    function resetModel() {
        listModel.clear();
        var items = updateModel();
        for(var i = 0; i< items.length; i++) {
            listModel.append(items[i]);
        }
    }


    displaced: Transition {
        NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad }
    }

    PlasmaCore.DataSource {
        id: pmSource
        engine: "powermanagement"
        connectedSources: ["PowerDevil"]
    }

    model: DelegateModel {
        id: visualModel
        model: ListModel {
            id: listModel
        }

        delegate: MouseArea {
            id: delegateRoot

            width: units.gridUnit * 6
            height: units.gridUnit * 5

            property int visualIndex: DelegateModel.itemsIndex

            drag.target: button

            onReleased: {
                button.Drag.drop()
            }

            KickoffConfigurationButton {
                id: button

                icon: model.icon == "pm" ? (pmSource.data["PowerDevil"] && pmSource.data["PowerDevil"]["Is Lid Present"] ? "computer-laptop" : "computer") : model.icon
                text: model.text || ""

                name: model.name

                anchors {
                    horizontalCenter: parent.horizontalCenter;
                    verticalCenter: parent.verticalCenter
                }


                Drag.active: delegateRoot.drag.active && name != 'empty'
                Drag.source: delegateRoot
                Drag.hotSpot.x: 36
                Drag.hotSpot.y: 36

                onStateChanged: {
                    if(!Drag.active && sourceIndex != -1) {
                        sourceIndex = -1;
                        updateConfiguration();
                    }
                }

                states: [
                    State {
                        when: button.Drag.active
                        ParentChange {
                            target: button;
                            parent: root;
                        }

                        AnchorChanges {
                            target: button;
                            anchors.horizontalCenter: undefined;
                            anchors.verticalCenter: undefined;
                        }
                    }
                ]
            }

            DropArea {
                anchors { fill: parent; margins: 15 }

                onEntered: {
                    var source = drag.source.visualIndex;
                    var target = delegateRoot.visualIndex;
                    sourceIndex = drag.source.visualIndex;
                    if(!(previousCell[0] == source && previousCell[1] == target)) {
                        previousCell = [source, target];
                         if(source < 5 && target >= 5) {
                             visualModel.items.move(source, target);
                             visualModel.items.move(target == 9 ? 8 : 9, 4);
                         } else if (source >= 5 && target < 5) {
                             visualModel.items.move(source, target);
                             visualModel.items.move(5, 9);
                         } else {
                            visualModel.items.move(source, target);
                        }
                    }
                }
                onDropped: {
                    var targetIndex = drag.source.visualIndex;
                    updateConfiguration();
                    sourceIndex = -1;
                    previousCell = [-1, -1];
                }
            }
        }
    }
}
