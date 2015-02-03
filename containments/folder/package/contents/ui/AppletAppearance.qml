/*
 *   Copyright 2011-2013 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *   Copyright 2014 David Edmundson <davidedmundson@kde.org>
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
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

Item {
    id: appletItem

    anchors.rightMargin: -handleWidth*controlsOpacity

    property int handleWidth: iconSize
    property int minimumHandleHeight: 6 * (root.iconSize + 6) + margins.top + margins.bottom
    property int handleHeight: (height < minimumHandleHeight) ? minimumHandleHeight : height
    property string category

    property bool showAppletHandle: false
    property real controlsOpacity: (plasmoid.immutable || !showAppletHandle) ? 0 : 1
    property bool handleShown: true
    property string backgroundHints: "NoBackground"
    property bool hasBackground: false
    property bool handleMerged: (height > minimumHandleHeight)
    property bool animationsEnabled: false

    property int minimumWidth: Math.max(LayoutManager.cellSize.width,
                           appletContainer.minimumWidth +
                           appletItem.contents.anchors.leftMargin +
                           appletItem.contents.anchors.rightMargin)

    property int minimumHeight: Math.max(LayoutManager.cellSize.height,
                            appletContainer.minimumHeight +
                            appletItem.contents.anchors.topMargin +
                            appletItem.contents.anchors.bottomMargin)

    property alias applet: appletContainer.applet

    property Item contents: appletContainer
    property alias margins: plasmoidBackground.margins
    property alias imagePath: plasmoidBackground.imagePath

    visible: false

    onMinimumWidthChanged: appletItem.width = Math.max(minimumWidth, width);
    onMinimumHeightChanged: appletItem.height = Math.max(minimumHeight, height);


    //FIXME: this delay is because backgroundHints gets updated only after a while in qml applets
    Timer {
        id: appletTimer
        interval: 50
        repeat: false
        running: false
        onTriggered: {
            updateBackgroundHints();
            applet.parent = appletContainer;
            applet.anchors.fill = appletContainer;
        }
    }

    function updateBackgroundHints() {
        hasBackground = (applet.backgroundHints != "NoBackground");
        if (applet.backgroundHints == 1) {
            appletItem.imagePath = "widgets/background";
            backgroundHints = "StandardBackground";
        } else if (applet.backgroundHints == 2) {
            appletItem.imagePath = "widgets/translucentbackground"
            backgroundHints = "TranslucentBackground";
        } else if (applet.backgroundHints == 0) {
            appletItem.imagePath = ""
            backgroundHints = "NoBackground";
        } else {
            backgroundHints = "DefaultBackground";
            appletItem.imagePath = "widgets/background";
        }
        //print("Backgroundhints changed: " + appletItem.imagePath);
    }

    Rectangle { color: Qt.rgba(0,0,0,0); border.width: 3; border.color: "white"; opacity: 0.5; visible: debug; anchors.fill: parent; }

    KQuickControlsAddons.MouseEventListener {
        id: mouseListener

        anchors {
            fill: parent
        }
        z: 10

        hoverEnabled: true

        onPressAndHold: {
            if (root.pressAndHoldHandle && !plasmoid.immutable) {
                //hoverTracker.interval = 400;
                hoverTracker.restart();
            }
        }

        onContainsMouseChanged: {

            animationsEnabled = true;
            //print("Mouse is " + containsMouse);
            if (!plasmoid.immutable) {
                if (!root.pressAndHoldHandle) {
                    hoverTracker.interval = root.handleDelay;
                    hoverTracker.restart();
                }
            }
        }

        Timer {
            id: hoverTracker
            repeat: false
            interval: handleDelay
            onTriggered: {
                if (mouseListener.containsMouse || (appletHandle.item && (appletHandle.item.containsMouse || appletHandle.item.pressed))) {
                    showAppletHandle = true;
                } else {
                    showAppletHandle = false;
                }
            }
        }
        Rectangle { color: Qt.rgba(0,0,0,0); border.width: 3; border.color: "red"; opacity: 0.5; visible: debug; anchors.fill: parent; }


        Item {
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom; }
            width: parent.width+handleWidth;

            z: mouseListener.z + 4

            PlasmaCore.FrameSvgItem {
                id: plasmoidBackground
                visible: backgroundHints != "NoBackground"
                imagePath: "widgets/background"
                anchors { left: parent.left; top: parent.top; bottom: parent.bottom; }
                width: (showAppletHandle && handleMerged) ? parent.width : parent.width-handleWidth;
                smooth: true

                Behavior on width {
                    enabled: animationsEnabled
                    NumberAnimation {
                        duration: units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            Connections {
                target: plasmoid
                onImmutableChanged: {
//                     print(" TB dragMouseArea.visible: " + plasmoid.immutable)
                    dragMouseArea.visible = !plasmoid.immutable;
                    showAppletHandle = false;
                }
                onAppletRemoved: {
//                     print("Applet removed Applet-" + applet.id)
                    if (applet.id == appletItem.applet.id) {
//                         print("Destroying Applet-" + applet.id)
                        LayoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)
                        //applet.action("remove").trigger();
                        //appletItem.destroy()
                        appletItem.destroy();
                    }
                }
            }
            Connections {
                target: applet
                onBusyChanged: {
                    if (applet.busy) {
                        busyLoader.source = "BusyOverlay.qml"
                    } else if (busyLoader.item && typeof(busyLoader.item) != "undefined") {
                        busyLoader.item.disappear();
                    }
                }
                onBackgroundHintsChanged: {
//                     print("plasmoid.backgroundHintsChanged");
                    updateBackgroundHints();
                }
            }

            MouseArea {
                id: dragMouseArea

                anchors.fill: parent
                z: appletContainer.z - 2
                visible: !plasmoid.immutable

                property int zoffset: 1000
                drag.target: appletItem

                onPressed: {
                    appletItem.z = appletItem.z + zoffset;
                    animationsEnabled = false
                    mouse.accepted = true
                    var x = Math.round(appletItem.x/LayoutManager.cellSize.width)*LayoutManager.cellSize.width
                    var y = Math.round(appletItem.y/LayoutManager.cellSize.height)*LayoutManager.cellSize.height
                    LayoutManager.setSpaceAvailable(x, y, appletItem.width, appletItem.height, true)

                    placeHolder.syncWithItem(appletItem)
                    placeHolderPaint.opacity = root.haloOpacity;
                }
                onPositionChanged: {
                    var pos = mapToItem(root, mouse.x, mouse.y);
                    var newCont = plasmoid.containmentAt(pos.x, pos.y);

                    if (newCont && newCont != plasmoid) {
                        var newPos = newCont.mapFromApplet(plasmoid, pos.x, pos.y);
                        newCont.addApplet(appletItem.applet, newPos.x, newPos.y);
                        placeHolderPaint.opacity = 0;
                    } else {
                        placeHolder.syncWithItem(appletItem);
                    }
                }
                onReleased: {
                    appletItem.z = appletItem.z - zoffset;
                    repositionTimer.running = false
                    placeHolderPaint.opacity = 0
                    animationsEnabled = true
                    LayoutManager.positionItem(appletItem)
                    LayoutManager.save()
                }
            }

            Item {
                id: appletContainer
                anchors {
                    fill: parent
                    leftMargin: plasmoidBackground.margins.left
                    rightMargin: plasmoidBackground.margins.right + handleWidth
                    topMargin: plasmoidBackground.margins.top
                    bottomMargin: plasmoidBackground.margins.bottom
                }
                z: mouseListener.z+1

                property QtObject applet

                property var minimumSize: {
                    var layout;
                    var size = new Object;
                    size['width'] = 0;
                    size['height'] = 0;

                    if (!applet) {
                        return size;
                    }

                    layout = applet.Layout

                    if (layout) {
                        size.width = layout.minimumWidth;
                        size.height = layout.minimumHeight;
                    }

                    return size;
                }

                property int minimumWidth: minimumSize.width;
                property int minimumHeight: minimumSize.height;

                function appletDestroyed() {
//                     print("Applet DESTROYED.");
                    LayoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)
                    applet.action("remove").trigger();
                    appletItem.destroy()
                }

                onAppletChanged: {
                    if (applet) {
                        appletTimer.running = true;
                    }
                }
                Connections {
                    target: appletHandle.item
                    onRemoveApplet: {
                        killAnim.running = true;
                    }
                }
                Behavior on opacity {
                    NumberAnimation {
                        duration: units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
                SequentialAnimation {
                    id: killAnim
                    PlasmaExtras.DisappearAnimation {
                        targetItem: appletItem
                    }
                    ScriptAction {
                        script: appletContainer.appletDestroyed()
                    }
                }
                Loader {
                    id: busyLoader
                    anchors.centerIn: parent
                    z: appletContainer.z + 1
                }
//                 Rectangle { color: "green"; opacity: 0.3; visible: debug; anchors.fill: parent; }
                Component.onCompleted: PlasmaExtras.AppearAnimation {
                    targetItem: appletItem
                }
            }

            Loader {
                id: appletHandle
                z: appletContainer.z + 1
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: plasmoidBackground.right
                    rightMargin: appletItem.margins.right
                }
                Connections {
                    target: appletItem
                    onShowAppletHandleChanged: {
                        if (appletItem.showAppletHandle && appletHandle.source == "") {
                            //print("Loading applethandle ");
                            appletHandle.source = "AppletHandle.qml";
                        }
                    }
                }

            }

//             Rectangle { color: "orange"; opacity: 0.1; visible: debug; anchors.fill: parent; }
        }
    }

    Behavior on controlsOpacity {
        NumberAnimation {
            duration: units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on x {
        enabled: animationsEnabled
        NumberAnimation {
            duration: units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on y {
        enabled: animationsEnabled
        NumberAnimation {
            duration: units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on width {
        enabled: animationsEnabled
        NumberAnimation {
            id: widthAnimation
            duration: units.longDuration
            easing.type: Easing.InOutQuad
        }
    }
    Behavior on height {
        enabled: animationsEnabled
        NumberAnimation {
            id: heightAnimation
            duration: units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    Component.onCompleted: {
        layoutTimer.running = true
        layoutTimer.restart()
        visible = false
        // restore rotation
    }
}
