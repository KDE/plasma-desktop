/*
 *  Copyright 2019 Marco Martin <mart@kde.org>
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

import QtQuick 2.12
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

import org.kde.plasma.private.containmentlayoutmanager 1.0 as ContainmentLayoutManager

ContainmentLayoutManager.ConfigOverlayWithHandles {
    id: overlay

    readonly property int iconSize: touchInteraction ? units.iconSizes.medium : units.iconSizes.small
    PlasmaCore.Svg {
        id: configIconsSvg
        imagePath: "widgets/configuration-icons"
    }

    SequentialAnimation {
        id: removeAnim
        NumberAnimation {
            target: overlay.itemContainer
            property: "scale"
            from: 1
            to: 0
            duration: units.longDuration
            easing.type: Easing.InOutQuad
        }
        ScriptAction {
            script: {
                appletContainer.applet.action("remove").trigger();
                appletContainer.editMode = false;
            }
        }
    }
    PlasmaCore.FrameSvgItem {
        id: frame
        anchors.verticalCenter: parent.verticalCenter
        x: overlay.rightAvailableSpace > width + units.gridUnit
            ? parent.width + units.gridUnit
            : -width - units.gridUnit

        // This MouseArea is used to block input between the applet and the handle, to not make it steal by other applets
        MouseArea {
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            z: -1
            x: overlay.rightAvailableSpace > parent.width + units.gridUnit ? -units.gridUnit : 0
            width: units.gridUnit + parent.width
            hoverEnabled: true
        }
        transform: Translate {
            x: open ? 0 : (overlay.rightAvailableSpace > frame.width + units.gridUnit ? -frame.width : frame.width)

            Behavior on x {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
        width: layout.implicitWidth + margins.left + margins.right
        height: Math.max(layout.implicitHeight + margins.top + margins.bottom, parent.height)
        imagePath: "widgets/background"

        ColumnLayout {
            id: layout
            anchors {
                fill: parent
                topMargin: parent.margins.top
                leftMargin: parent.margins.left
                bottomMargin: parent.margins.bottom
                rightMargin: parent.margins.right
            }

            ActionButton {
                id: rotateButton
                svg: configIconsSvg
                elementId: "rotate"
                toolTip: !rotateHandle.pressed ? i18n("Rotate") : ""
                iconSize: overlay.iconSize
                action: (applet) ? applet.action("rotate") : null
                down: !rotateHandle.pressed
                Component.onCompleted: {
                    if (action && typeof(action) != "undefined") {
                        action.enabled = true
                    }
                }
                MouseArea {
                    id: rotateHandle
                    anchors.fill: parent

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
                        var mousePos = overlay.itemContainer.parent.mapFromItem(rotateButton, x, y);
                        var centerPos = overlay.itemContainer.parent.mapFromItem(overlay.itemContainer, overlay.itemContainer.width/2, overlay.itemContainer.height/2);

                        mousePos.x -= centerPos.x;
                        mousePos.y -= centerPos.y;
                        return mousePos;
                    }

                    onPressed: {
                        mouse.accepted = true
                        startRotation = overlay.itemContainer.rotation;
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
                        overlay.itemContainer.rotation = newRotation;
                    }
                    onReleased: {
                        // save rotation
    //                    print("saving...");
                        appletsLayout.save();
                    }
                }
            }

            ActionButton {
                svg: configIconsSvg
                elementId: "configure"
                iconSize: overlay.iconSize
                visible: (qAction && typeof(qAction) != "undefined") ? qAction.enabled : false
                qAction: (applet) ? applet.action("configure") : null
                Component.onCompleted: {
                    if (qAction && typeof(qAction) != "undefined") {
                        qAction.enabled = true
                    }
                }
            }

            ActionButton {
                svg: configIconsSvg
                elementId: "maximize"
                iconSize: overlay.iconSize
                toolTip: i18n("Open Externally")
                visible: (qAction && typeof(qAction) != "undefined") ? qAction.enabled : false
                qAction: (applet) ? applet.action("run associated application") : null
                Component.onCompleted: {
                    if (qAction && typeof(qAction) != "undefined") {
                        qAction.enabled = true
                    }
                }
            }
            ActionButton {
                svg: configIconsSvg
                elementId: "showbackground"
                toolTip: checked ? i18n("Hide Background") : i18n("Show Background")
                iconSize: overlay.iconSize
                visible: (applet.backgroundHints & PlasmaCore.Types.ConfigurableBackground)
                checked: applet.effectiveBackgroundHints & PlasmaCore.Types.StandardBackground || applet.effectiveBackgroundHints & PlasmaCore.Types.TranslucentBackground
                checkable: true
                onClicked: {
                    if (checked) {
                        if (applet.backgroundHints & PlasmaCore.Types.StandardBackground || applet.backgroundHints & PlasmaCore.Types.TranslucentBackground) {
                            applet.userBackgroundHints = applet.backgroundHints;
                        } else {
                            applet.userBackgroundHints = PlasmaCore.Types.StandardBackground;
                        }
                    } else {
                        if (applet.backgroundHints & PlasmaCore.Types.ShadowBackground || applet.backgroundHints & PlasmaCore.Types.NoBackground) {
                            applet.userBackgroundHints = applet.backgroundHints;
                        } else {
                            applet.userBackgroundHints = PlasmaCore.Types.ShadowBackground;
                        }
                    }
                }
            }

            MouseArea {
                drag.target: overlay.itemContainer
                Layout.minimumHeight: units.gridUnit * 3
                Layout.fillHeight: true
                Layout.fillWidth: true
                cursorShape: containsPress? Qt.DragMoveCursor : Qt.OpenHandCursor
                hoverEnabled: true
                onPressed: appletsLayout.releaseSpace(overlay.itemContainer);
                onPositionChanged: {
                    if (!pressed) {
                        return;
                    }
                    appletsLayout.showPlaceHolderForItem(overlay.itemContainer);
                    var dragPos = mapToItem(overlay.itemContainer, mouse.x, mouse.y);
                    overlay.itemContainer.userDrag(Qt.point(overlay.itemContainer.x, overlay.itemContainer.y), dragPos);
                }
                onReleased: {
                    appletsLayout.hidePlaceHolder();
                    appletsLayout.positionItem(overlay.itemContainer);
                }
            }

            ActionButton {
                id: closeButton
                svg: configIconsSvg
                elementId: "delete"
                toolTip: i18n("Remove")
                iconSize: overlay.iconSize
                visible: {
                    if (!applet) {
                        return false;
                    }
                    var a = applet.action("remove");
                    return (a && typeof(a) != "undefined") ? a.enabled : false;
                }
                // we don't set action, since we want to catch the button click,
                // animate, and then trigger the "remove" action
                // Triggering the action is handled in the overlay.itemContainer, we just
                // emit a signal here to avoid the applet-gets-removed-before-we-
                // can-animate it race condition.
                onClicked: {
                    removeAnim.restart();
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

