// -*- coding: iso-8859-1 -*-
/*
 *   Copyright 2011-2013 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.draganddrop 2.0 as DragDrop

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

DragDrop.DropArea {
    id: root

    property Item toolBox

    property bool debug: false
    property int handleDelay: 800
    property real haloOpacity: 0.5
    property bool pressAndHoldHandle: false

    property int iconSize: 16
    property int iconWidth: iconSize
    property int iconHeight: iconWidth

    onIconHeightChanged: updateGridSize()

    anchors {
        leftMargin: plasmoid.availableScreenRect ? plasmoid.availableScreenRect.x : 0
        topMargin: plasmoid.availableScreenRect ? plasmoid.availableScreenRect.y : 0

        rightMargin: plasmoid.availableScreenRect && parent ? parent.width - (plasmoid.availableScreenRect.x + plasmoid.availableScreenRect.width) : 0
        bottomMargin: plasmoid.availableScreenRect && parent ? parent.height - (plasmoid.availableScreenRect.y + plasmoid.availableScreenRect.height) : 0
    }

    function updateGridSize()
    {
//         print("Updategridsize");
        LayoutManager.cellSize.width = root.iconWidth + toolBoxSvg.elementSize("left").width + toolBoxSvg.elementSize("right").width
        LayoutManager.cellSize.height = root.iconHeight + toolBoxSvg.elementSize("top").height + toolBoxSvg.elementSize("bottom").height;
        LayoutManager.defaultAppletSize.width = LayoutManager.cellSize.width * 6;
        LayoutManager.defaultAppletSize.height = LayoutManager.cellSize.height * 6;
        layoutTimer.restart()
    }

    function addApplet(applet, x, y) {
        var component = Qt.createComponent("AppletAppearance.qml");
        var e = component.errorString();
        if (e != "") {
            print("Error loading AppletAppearance.qml: " + component.errorString());
        }

        var container = component.createObject(resultsFlow)

        applet.parent = container
        applet.visible = true;

        container.category = "Applet-"+applet.id;
        var config = LayoutManager.itemsConfig[container.category];

        //we have it in the config
        if (config !== undefined && config.width !== undefined &&
            config.height !== undefined &&
            config.width > 0 && config.height > 0) {
            container.width = config.width;
            container.height = config.height;
        //we have a default
        } else if (applet.width > 0 && applet.height > 0) {
            container.width = applet.width;
            container.height = applet.height;
        //give up, assign the global default
        } else {
            container.width = LayoutManager.defaultAppletSize.width;
            container.height = LayoutManager.defaultAppletSize.height;
        }

        container.applet = applet;
        //coordinated passed by param?
        if ( x >= 0 && y >= 0) {
            if (x + container.width > root.width) {
                x = root.width - container.width - 10;
            }
            if (y + container.height > root.height) {
                x = root.height - container.height;
            }
            container.x = x;
            container.y = y;
        //coordinates stored?
        } else if (config !== undefined && config.x !== undefined && config.y !== undefined &&
            config.x >= 0 && config.y >= 0) {
            container.x = config.x;
            container.y = config.y;
        }

        //rotation sotired and significative?
        if (config !== undefined && config.rotation !== undefined &&
            (config.rotation > 5 || config.rotation < -5)) {
            container.rotation = config.rotation;
        }

        LayoutManager.itemGroups[container.category] = container;
//         print("Applet " + container.category + applet.title + " added at" + container.x + " " + container.y);

        if (container.x >= 0 && container.y >= 0) {
            LayoutManager.positionItem(container);
        }
    }

    onDragEnter: {
        placeHolder.width = LayoutManager.defaultAppletSize.width;
        placeHolder.height = LayoutManager.defaultAppletSize.height;
        placeHolder.x = event.x - placeHolder.width / 2;
        placeHolder.y = event.y - placeHolder.width / 2;
        LayoutManager.positionItem(placeHolder);
        LayoutManager.setSpaceAvailable(placeHolder.x, placeHolder.y, placeHolder.width, placeHolder.height, true);
        placeHolderPaint.opacity = root.haloOpacity;
    }

    onDragMove: {
        placeHolder.width = LayoutManager.defaultAppletSize.width;
        placeHolder.height = LayoutManager.defaultAppletSize.height;
        placeHolder.x = event.x - placeHolder.width / 2;
        placeHolder.y = event.y - placeHolder.width / 2;
        LayoutManager.positionItem(placeHolder);
        LayoutManager.setSpaceAvailable(placeHolder.x, placeHolder.y, placeHolder.width, placeHolder.height, true);
        placeHolderPaint.opacity = root.haloOpacity;
    }

    onDragLeave: {
        placeHolderPaint.opacity = 0;
    }

    onDrop: {
        placeHolderPaint.opacity = 0;
        plasmoid.processMimeData(event.mimeData, event.x - placeHolder.width / 2, event.y - placeHolder.height / 2);
    }


    Connections {
        target: plasmoid

        onAppletAdded: {
            addApplet(applet, x, y);
            //clean any eventual invalid chunks in the config
            LayoutManager.save();
        }

        onAppletRemoved: {
            //clean any eventual invalid chunks in the config
//             console.log("Applet removed");
            LayoutManager.removeApplet(applet);
            LayoutManager.save();
        }
    }

    PlasmaCore.Svg {
        id: toolBoxSvg
        imagePath: "widgets/toolbox"
        property int rightBorder: elementSize("right").width
        property int topBorder: elementSize("top").height
        property int bottomBorder: elementSize("bottom").height
        property int leftBorder: elementSize("left").width
    }
    PlasmaCore.Svg {
        id: configIconsSvg
        imagePath: "widgets/configuration-icons"
    }

    Item {
        id: resultsFlow
        anchors.fill: parent

        anchors {
            top: parent.top
            topMargin: 5
            horizontalCenter: parent.horizontalCenter
        }

        //This is just for event compression when a lot of boxes are created one after the other
        Timer {
            id: layoutTimer
            repeat: false
            running: false
            interval: 100
            onTriggered: {
                LayoutManager.resetPositions()
                for (var i=0; i<resultsFlow.children.length; ++i) {
                    var child = resultsFlow.children[i]
                    if (child.enabled) {
                        if (LayoutManager.itemsConfig[child.category]) {
                            var rect = LayoutManager.itemsConfig[child.category]
                            child.x = rect.x
                            child.y = rect.y
                            child.width = rect.width
                            child.height = rect.height
                            child.rotation = rect.rotation
                        } else {
                            child.x = 0
                            child.y = 0
                            child.width = Math.min(470, 32+child.categoryCount*140)
                        }
                        child.visible = true
                        LayoutManager.positionItem(child)
                    } else {
                        child.visible = false
                    }
                }
                LayoutManager.save()
            }
        }
    }

    Item {
        anchors.fill: resultsFlow
        z: 0

        Item {
            id: placeHolder

            x: -10000 // move offscreen initially to avoid flickering
            width: 100
            height: 100

            property bool animationsEnabled
            property int minimumWidth
            property int minimumHeight
            property Item syncItem

            function syncWithItem(item) {
                syncItem = item
                minimumWidth = item.minimumWidth
                minimumHeight = item.minimumHeight
                repositionTimer.running = true
                if (placeHolderPaint.opacity < 1) {
                    placeHolder.delayedSyncWithItem()
                }
            }

            function delayedSyncWithItem() {
                placeHolder.x = placeHolder.syncItem.x
                placeHolder.y = placeHolder.syncItem.y
                placeHolder.width = placeHolder.syncItem.width
                placeHolder.height = placeHolder.syncItem.height
                //only positionItem here, we don't want to save
                LayoutManager.positionItem(placeHolder)
                LayoutManager.setSpaceAvailable(placeHolder.x, placeHolder.y, placeHolder.width, placeHolder.height, true)
            }

            Timer {
                id: repositionTimer
                interval: 100
                repeat: false
                running: false
                onTriggered: placeHolder.delayedSyncWithItem()
            }
        }

        PlasmaComponents.Highlight {
            id: placeHolderPaint

            x: placeHolder.x + (root.iconSize/2)
            y: placeHolder.y + (root.iconSize/2)
            width: placeHolder.width + (root.iconSize/2)
            height: placeHolder.height - root.iconSize
            z: 0
            visible: false

            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    Component.onCompleted: {
        placeHolderPaint.opacity = 0;
        placeHolderPaint.visible = true;
//         print("Containment completed.");
        LayoutManager.resultsFlow = resultsFlow
        LayoutManager.plasmoid = plasmoid
        updateGridSize()
        //plasmoid.containmentType = "DesktopContainment"

        LayoutManager.restore()

        for (var i = 0; i < plasmoid.applets.length; ++i) {
            var applet = plasmoid.applets[i]
            addApplet(applet, -1, -1);
        }
        //clean any eventual invalid chunks in the config
        LayoutManager.save();
    }
}
