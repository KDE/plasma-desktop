/***************************************************************************
 *   Copyright 2012 by Sebastian Kügler <sebas@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

Item {
    id: toolBoxItem

    property bool showing//: state != "collapsed"
    property int expandedWidth: 240
    property int expandedHeight: 240

    width: childrenRect.width
    height: childrenRect.height
    transformOrigin: {
        if (toolBoxButton.state == "topright") {
            return Item.TopRight;
        } else if (toolBoxButton.state == "right") {
            return Item.Right;
        } else if (toolBoxButton.state == "bottomright") {
            return Item.BottomRight;
        } else if (toolBoxButton.state == "bottom") {
            return Item.Bottom;
        } else if (toolBoxButton.state == "bottomleft") {
            return Item.BottomLeft;
        } else if (toolBoxButton.state == "left") {
            return Item.Left;
        } else if (toolBoxButton.state == "topleft") {
            return Item.TopLeft;
        } else if (toolBoxButton.state == "top") {
            return Item.Top;
        }
    }

    state: "collapsed"

    PlasmaCore.DataSource {
        id: dataEngine
        engine: "powermanagement"
        connectedSources: ["PowerDevil"]
    }

    onShowingChanged: {
        print("TB showing changed to " + showing);
        state = showing ? "expanded" : "collapsed";
    }

    function performOperation(what) {
        var service = dataEngine.serviceForSource("PowerDevil");
        var operation = service.operationDescription(what);
        return service.startOperationCall(operation);
    }

    function lockScreen() {
        print("TB locking...");
        performOperation("lockScreen");
    }

    function lockWidgets(lock) {
        plasmoid.action("lock").trigger();
    }

    function logout() {
        print("TB shutdown...");
        performOperation("requestShutDown");
    }

    function containmentSettings() {
        plasmoid.action("configure").trigger();
    }

    function shortcutSettings() {
        print("FIXME: implement shortcut settings");
    }

    function showWidgetsExplorer() {
        plasmoid.action("add widgets").trigger();
    }

    function showActivities() {
        print("TB FIXME: Show Activity Manager");
    }


    PlasmaCore.FrameSvgItem {
        id: toolBoxFrame

        width: actionList.width + toolBoxFrame.margins.left + toolBoxFrame.margins.right
        height: actionList.height + toolBoxFrame.margins.top + toolBoxFrame.margins.bottom
        z: 1000

        property Item highlight
        property Item currentItem: null
        onCurrentItemChanged: {
            if (!currentItem) {
                return
            } else if (highlight == toolBoxHighlight1) {
                toolBoxHighlight1.opacity = 0;
                highlight = toolBoxHighlight2;
                toolBoxHighlight2.item = currentItem;
                toolBoxHighlight2.opacity = 1;
            } else {
                toolBoxHighlight2.opacity = 0;
                highlight = toolBoxHighlight1;
                toolBoxHighlight1.item = currentItem;
                toolBoxHighlight1.opacity = 1;
            }
        }

        imagePath: "widgets/background"

        Behavior on height {
            NumberAnimation {
                duration: units.shortDuration
                easing.type: Easing.OutQuad
            }
        }

        Timer {
            id: exitTimer
            interval: 200
            running: true
            repeat: false
            onTriggered: { 
                if (toolBoxFrame.highlight) {
                    toolBoxFrame.highlight.opacity = 0;
                }
                toolBoxFrame.currentItem = null;
            }
        }

        Column {
            id: actionList

            x: parent.x + toolBoxFrame.margins.left
            y: parent.y + toolBoxFrame.margins.top

            Repeater {
                id: unlockedList
                model: plasmoid.actions
                delegate: ActionDelegate {
                    actionIcon: modelData.icon
                    objectName: modelData.objectName
                }
            }

            ActionDelegate {
                label: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Leave")
                actionIcon: "system-log-out"
                objectName: "leave"
                onTriggered: logout();
            }
        }

        PlasmaComponents.Highlight {
            id: toolBoxHighlight1
            property Item item

            opacity: 0
            x: (item != null) ? item.x + toolBoxFrame.margins.left - units.smallSpacing : toolBoxFrame.margins.left
            y: (item != null) ? item.y + toolBoxFrame.margins.top - units.smallSpacing : toolBoxFrame.margins.top
            width: actionList.width + units.smallSpacing * 2
            height: (item != null) ? item.height + units.smallSpacing * 2 : 0

            Behavior on opacity {
                NumberAnimation {
                    duration: units.shortDuration * 3
                    easing.type: Easing.InOutQuad
                }
            }
        }
        PlasmaComponents.Highlight {
            id: toolBoxHighlight2
            property Item item

            opacity: 0
            x: (item != null) ? item.x + toolBoxFrame.margins.left - units.smallSpacing : toolBoxFrame.margins.left
            y: (item != null) ? item.y + toolBoxFrame.margins.top - units.smallSpacing : toolBoxFrame.margins.top
            width: actionList.width + units.smallSpacing * 2
            height: (item != null) ? item.height + units.smallSpacing * 2 : 0

            Behavior on opacity {
                NumberAnimation {
                    duration: units.shortDuration * 3
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    states: [
        State {
            name: "expanded"
            PropertyChanges { target: toolBoxItem; opacity: 1.0; scale: 1; enabled: true}
        },
        State {
            name: "collapsed"
            PropertyChanges { target: toolBoxItem; opacity: 0; scale: 0.8; enabled: false}
        }
    ]

    transitions: [
        Transition {
            ParallelAnimation {
                NumberAnimation {
                    target: toolBoxItem
                    properties: "opacity"
                    easing.type: Easing.InExpo
                    duration: units.longDuration
                }
                NumberAnimation {
                    target: toolBoxItem
                    properties: "scale"
                    easing.type: Easing.InExpo
                    duration: units.shortDuration * 3
                }
            }
        }
    ]
}
