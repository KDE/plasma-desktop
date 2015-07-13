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
import org.kde.plasma.components 2.0 as PlasmaComponents

KQuickControlsAddons.MouseEventListener {
    id: appletHandle

    opacity: appletItem.controlsOpacity
    visible: opacity > 0
    width: appletItem.handleWidth
    height: Math.max(appletItem.height - appletItem.margins.top - appletItem.margins.bottom, buttonColumn.implicitHeight)
    hoverEnabled: true
    onContainsMouseChanged: {
        if (!containsMouse && !pressed) {
            hoverTracker.restart()
        }
    }
    onReleased: {
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
//             verticalCenter: parent.verticalCenter

            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }

        smooth: true
        imagePath: (backgroundHints == "NoBackground" || !handleMerged) ? "widgets/background" : ""
    }

     PlasmaComponents.Label {
        id: toolTipDelegate

        width: contentWidth
        height: contentHeight

        property Item toolTip

        text: (toolTip != null) ? toolTip.mainText : ""
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
            mainText: i18n("Resize")
            active: !resizeHandle.pressed

            MouseArea {
                id: resizeHandle
                anchors {
                    fill: parent
                    margins: -buttonMargin
                }

                property int startX
                property int startY

                onPressed: {
                    parent.hideToolTip();
                    mouse.accepted = true
                    animationsEnabled = false;
                    startX = mouse.x;
                    startY = mouse.y;
                    root.layoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)
                }
                onPositionChanged: {
                    appletItem.width = Math.max(appletItem.minimumWidth + appletHandle.width, appletItem.width + mouse.x-startX);

                    var oldBottom = appletItem.y + appletItem.height;
                    appletItem.height = Math.max(appletItem.minimumHeight, appletItem.height + startY-mouse.y)
                    appletItem.y = oldBottom - appletItem.height;
                }
                onReleased: {
                    animationsEnabled = true
                    root.layoutManager.positionItem(appletItem)
                    root.layoutManager.save()
                    root.layoutManager.setSpaceAvailable(appletItem.x, appletItem.y, widthAnimation.to, heightAnimation.to, false)
                }
//                 Rectangle { color: "blue"; opacity: 0.4; visible: debug; anchors.fill: parent; }
            }
        }
        ActionButton {
            id: rotateButton
            svg: configIconsSvg
            elementId: "rotate"
            mainText: i18n("Rotate")
            iconSize: root.iconSize
            action: (applet) ? applet.action("rotate") : null
            active: !rotateHandle.pressed
            Component.onCompleted: {
                if (action && typeof(action) != "undefined") {
                    action.enabled = true
                }
            }
            MouseArea {
                id: rotateHandle
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
                    parent.hideToolTip();
                    mouse.accepted = true
                    animationsEnabled = false;
                    startRotation = appletItem.rotation;
                    root.layoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)

                    startCenterRelativeAngle = pointAngle(centerRelativePos(mouse.x, mouse.y));
                }
                onPositionChanged: {

                    var rot = startRotation%360;
                    var snap = 4;
                    var newRotation = Math.round(pointAngle(centerRelativePos(mouse.x, mouse.y)) - startCenterRelativeAngle + startRotation);

                    if (newRotation < 0) {
                        newRotation = newRotation + 360;
                    } else if (newRotation >= 360) {
                        newRotation = newRotation % 360;
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
                    //print("Start: " + startRotation  + " new: " + newRotation);
                    appletItem.rotation = newRotation;
                }
                onReleased: {
                    // save rotation
//                    print("saving...");
                    root.layoutManager.saveItem(appletItem);
                }
//                 Rectangle { color: "red"; opacity: 0.6; visible: debug; anchors.fill: parent; }
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
            elementId: "maximize"
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
            cursorShape: Qt.DragMoveCursor

            onPressed: {
                appletItem.z = appletItem.z + zoffset;
                animationsEnabled = false
                mouse.accepted = true
                var x = Math.round(appletItem.x / root.layoutManager.cellSize.width) * root.layoutManager.cellSize.width
                var y = Math.round(appletItem.y / root.layoutManager.cellSize.height) * root.layoutManager.cellSize.height
                root.layoutManager.setSpaceAvailable(x, y, appletItem.width, appletItem.height, true)

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
                root.layoutManager.positionItem(appletItem)
                root.layoutManager.save()
            }
        }

        Item {
            Layout.minimumWidth: closeButton.width
            Layout.minimumHeight: closeButton.height

            ActionButton {
                id: closeButton
                svg: configIconsSvg
                elementId: "close"
                mainText: i18n("Remove")
                iconSize: root.iconSize
                visible: {
                    if (!applet) {
                        return false;
                    }
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
}
