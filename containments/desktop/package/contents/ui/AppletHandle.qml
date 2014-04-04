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
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

KQuickControlsAddons.MouseEventListener {
    id: appletHandle

    opacity: appletItem.controlsOpacity
    visible: opacity > 0
    width: appletItem.handleWidth
    height: Math.max(appletItem.height - appletItem.margins.top - appletItem.margins.bottom, buttonColumn.implicitHeight)
    hoverEnabled: true
    onContainsMouseChanged: {
        if (!containsMouse) {
            hoverTracker.restart()
        }
    }
    //z: dragMouseArea.z + 1

    property int buttonMargin: 6
    property int minimumHeight:  6 * (root.iconSize + buttonMargin)

    signal removeApplet

    transform: Translate {
        x: handleMerged ? 0 : controlsOpacity * appletHandle.width
    }

    PlasmaCore.FrameSvgItem {
        id: noBackgroundHandle
        visible: controlsOpacity > 0
        z: plasmoidBackground.z - 10

        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter

            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }

        smooth: true
        imagePath: (backgroundHints == "NoBackground" || !handleMerged) ? "widgets/background" : ""
    }

    ColumnLayout {
        id: buttonColumn
        width: handleWidth

        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        spacing: buttonMargin*2
        ActionButton {
            svg: configIconsSvg
            elementId: "size-diagonal-tr2bl"
            iconSize: root.iconSize
            visible: (action && typeof(action) != "undefined") ? action.enabled : false
            action: (applet) ? applet.action("configure") : null
            Component.onCompleted: {
                if (action && typeof(action) != "undefined") {
                    action.enabled = true
                }
            }

            MouseArea {
                id: resizeHandle
                anchors {
                    fill: parent
                    margins: -buttonMargin
                }

                property int startX
                property int startY

                onPressed: {
                    mouse.accepted = true
                    animationsEnabled = false;
                    startX = mouse.x;
                    startY = mouse.y;
                    LayoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)
                }
                onPositionChanged: {
                    appletItem.width = Math.max(appletItem.minimumWidth, appletItem.width + mouse.x-startX);

                    var oldBottom = appletItem.y + appletItem.height;
                    appletItem.height = Math.max(appletItem.minimumHeight, appletItem.height + startY-mouse.y)
                    appletItem.y = oldBottom - appletItem.height;
                }
                onReleased: {
                    animationsEnabled = true
                    LayoutManager.positionItem(appletItem)
                    LayoutManager.save()
                    LayoutManager.setSpaceAvailable(appletItem.x, appletItem.y, widthAnimation.to, heightAnimation.to, false)
                }
                Rectangle { color: "blue"; opacity: 0.4; visible: debug; anchors.fill: parent; }
            }
        }
        ActionButton {
            id: rotateButton
            svg: configIconsSvg
            elementId: "rotate"
            iconSize: root.iconSize
            action: (applet) ? applet.action("rotate") : null
            Component.onCompleted: {
                if (action && typeof(action) != "undefined") {
                    action.enabled = true
                }
            }
            MouseArea {

                anchors {
                    fill: parent
                    margins: -buttonMargin
                }

                property int startRotation
                property real startCenterRelativeAngle;

                function pointAngle(pos) {
                    var r = Math.sqrt(pos.x * pos.x + pos.y * pos.y);
                    var cosine = pos.x / r;

                    if (pos.y >= 0) {
                        return Math.acos(cosine) * (180/Math.PI);
                    } else {
                        return -Math.acos(cosine) * (180/Math.PI);
                    }
                }

                function centerRelativePos(x, y) {
                    var mousePos = appletItem.parent.mapFromItem(rotateButton, x, y);
                    var centerPos = appletItem.parent.mapFromItem(appletItem, appletItem.width/2, appletItem.height/2);

                    mousePos.x -= centerPos.x;
                    mousePos.y -= centerPos.y;
                    return mousePos;
                }

                onPressed: {
                    mouse.accepted = true
                    animationsEnabled = false;
                    startRotation = appletItem.rotation;
                    LayoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)

                    startCenterRelativeAngle = pointAngle(centerRelativePos(mouse.x, mouse.y));
                }
                onPositionChanged: {

                    var rot = startRotation%360;
                    var snap = 4;
                    var newRotation = pointAngle(centerRelativePos(mouse.x, mouse.y)) - startCenterRelativeAngle + startRotation;

                    if (newRotation < 0) {
                        newRotation = newRotation + 360;
                    }

                    snapIt(0);
                    snapIt(90);
                    snapIt(180);
                    snapIt(270);

                    function snapIt(snapTo) {
                        if (newRotation > (snapTo - snap) && newRotation < (snapTo + snap)) {
                            newRotation = snapTo;
                        }
                    }
                    print("Start: " + startRotation  + " new: " + newRotation);
                    appletItem.rotation = newRotation;
                }
                onReleased: {
                    // save rotation
//                    print("saving...");
                    LayoutManager.saveItem(appletItem);
                }
                Rectangle { color: "red"; opacity: 0.6; visible: debug; anchors.fill: parent; }
            }
        }
        ActionButton {
            svg: configIconsSvg
            elementId: "configure"
            iconSize: root.iconSize
            visible: (action && typeof(action) != "undefined") ? action.enabled : false
            action: (applet) ? applet.action("configure") : null
            Component.onCompleted: {
                if (action && typeof(action) != "undefined") {
                    action.enabled = true
                }
            }
        }
        ActionButton {
            svg: configIconsSvg
            elementId: "size-diagonal-tr2bl" // FIXME should be maximize
            //elementId: "maximize"
            iconSize: root.iconSize
            visible: (action && typeof(action) != "undefined") ? action.enabled : false
            action: (applet) ? applet.action("run associated application") : null
            Component.onCompleted: {
                if (action && typeof(action) != "undefined") {
                    action.enabled = true
                }
            }
        }

        //spacer
        MouseArea {
            id: dragMouseArea

            implicitHeight: root.iconSize * 2
            Layout.fillWidth: true
            Layout.fillHeight: true
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
                placeHolder.syncWithItem(appletItem)
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

        ActionButton {
            svg: configIconsSvg
            elementId: "close"
            iconSize: root.iconSize
            visible: {
                var a = applet.action("remove");
                return (a && typeof(a) != "undefined") ? a.enabled : false;
            }
            // we don't set action, since we want to catch the button click,
            // animate, and then trigger the "remove" action
            // Triggering the action is handled in the appletItem, we just
            // emit a signal here to avoid the applet-gets-removed-before-we-
            // can-animate it race condition.
            onClicked: {
                appletHandle.removeApplet();
            }
            Component.onCompleted: {
                var a = applet.action("remove");
                if (a && typeof(a) != "undefined") {
                    a.enabled = true
                }
            }
        }
    }
}
