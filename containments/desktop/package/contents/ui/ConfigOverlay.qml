/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg


import org.kde.plasma.private.containmentlayoutmanager as ContainmentLayoutManager

ContainmentLayoutManager.ConfigOverlayWithHandles {
    id: overlay

    SequentialAnimation {
        id: removeAnim

        NumberAnimation {
            target: overlay.itemContainer
            property: "scale"
            from: 1
            to: 0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.InOutQuad
        }
        ScriptAction {
            script: {
                appletContainer.applet.plasmoid.internalAction("remove").trigger();
                appletContainer.editMode = false;
            }
        }
    }

    Item {
        id: actionOverlayItem
        opacity: rotateHandle.pressed ? 0 : 1

        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: {
            let widthDifference = Math.round((actionOverlayItem.width - overlay.width) / 2)
            if (widthDifference > 0) {
                if (widthDifference > overlay.leftAvailableSpace) {
                    return widthDifference - overlay.leftAvailableSpace
                }
                if (widthDifference > overlay.rightAvailableSpace) {
                    return overlay.rightAvailableSpace - widthDifference
                }
            }
            return 0
        }
        y: overlay.topAvailableSpace > height + Kirigami.Units.gridUnit
            ? -height - Kirigami.Units.gridUnit * 2
            : parent.height + Kirigami.Units.gridUnit * 2


        transform: Translate {
            y: overlay.open ? 0 : (overlay.topAvailableSpace > actionOverlayItem.height + Kirigami.Units.gridUnit ? -actionOverlayItem.height : actionOverlayItem.height)

            Behavior on y {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
        width: layout.implicitWidth + Kirigami.Units.gridUnit
        height: layout.implicitHeight + Kirigami.Units.gridUnit

        RowLayout {
            id: layout
            anchors {
                fill: parent
                topMargin: Kirigami.Units.gridUnit
                leftMargin: Kirigami.Units.gridUnit
                bottomMargin: Kirigami.Units.gridUnit
                rightMargin: Kirigami.Units.gridUnit
            }
            spacing: Kirigami.Units.gridUnit

            ActionButton {
                icon.name: "configure"
                text: i18nc("@action:button", "Configure")
                visible: qAction && qAction.enabled && (applet && applet.plasmoid.hasConfigurationInterface)
                qAction: applet ? applet.plasmoid.internalAction("configure") : null
                Component.onCompleted: {
                    if (qAction) {
                        qAction.enabled = true;
                    }
                }
            }

            ActionButton {
                icon.name: "show-background"
                text: checked ? i18nc("@action:button", "Hide Background") : i18nc("@action:button", "Show Background")
                toolTip: checked ? i18nc("@action:button tooltip hide widget background", "Hide Background") : i18nc("@action:button tooltip", "Show Background")
                visible: (applet.plasmoid.backgroundHints & PlasmaCore.Types.ConfigurableBackground)
                checked: applet.plasmoid.effectiveBackgroundHints & PlasmaCore.Types.StandardBackground || applet.plasmoid.effectiveBackgroundHints & PlasmaCore.Types.TranslucentBackground
                checkable: true
                onClicked: {
                    if (checked) {
                        if (applet.plasmoid.backgroundHints & PlasmaCore.Types.StandardBackground || applet.plasmoid.backgroundHints & PlasmaCore.Types.TranslucentBackground) {
                            applet.plasmoid.userBackgroundHints = applet.plasmoid.backgroundHints;
                        } else {
                            applet.plasmoid.userBackgroundHints = PlasmaCore.Types.StandardBackground;
                        }
                    } else {
                        if (applet.plasmoid.backgroundHints & PlasmaCore.Types.ShadowBackground || applet.plasmoid.backgroundHints & PlasmaCore.Types.NoBackground) {
                            applet.plasmoid.userBackgroundHints = applet.plasmoid.backgroundHints;
                        } else {
                            applet.plasmoid.userBackgroundHints = PlasmaCore.Types.ShadowBackground;
                        }
                    }
                }
            }

            ActionButton {
                id: closeButton
                icon.name: "edit-delete-remove"
                text: i18nc("@action:button", "Remove")
                toolTip: i18nc("@action:button tooltip remove widget", "Remove Applet")
                visible: {
                    if (!applet) {
                        return false;
                    }
                    var a = applet.plasmoid.internalAction("remove");
                    return a && a.enabled || false;
                }
                // we don't set action, since we want to catch the button click,
                // animate, and then trigger the "remove" action
                // Triggering the action is handled in the overlay.itemContainer, we just
                // Q_EMIT a signal here to avoid the applet-gets-removed-before-we-
                // can-animate it race condition.
                onClicked: {
                    removeAnim.restart();
                }
                Component.onCompleted: {
                    var a = applet.plasmoid.internalAction("remove");
                    if (a) {
                        a.enabled = true;
                    }
                }
            }
        }
    }

    Item {
        id: rotateItem

        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset: {
            let heightDifference = Math.round((rotateItem.height - overlay.height) / 2)
            if (heightDifference > 0) {
                if (heightDifference > overlay.topAvailableSpace) {
                    return heightDifference - overlay.topAvailableSpace
                }
                if (heightDifference > overlay.bottomAvailableSpace) {
                    return overlay.bottomAvailableSpace - heightDifference
                }
            }
            return 0
        }
        x: overlay.rightAvailableSpace > width + Kirigami.Units.gridUnit
            ? parent.width + Kirigami.Units.gridUnit
            : -width - Kirigami.Units.gridUnit

        transform: Translate {
            x: overlay.open ? 0 : (overlay.rightAvailableSpace > rotateItem.width + Kirigami.Units.gridUnit ? -rotateItem.width : rotateItem.width)

            Behavior on x {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        width: rotateLayout.implicitWidth + Kirigami.Units.gridUnit
        height: rotateLayout.implicitHeight + Kirigami.Units.gridUnit

        RowLayout {
            id: rotateLayout
            anchors {
                fill: parent
                topMargin: Kirigami.Units.gridUnit
                leftMargin: Kirigami.Units.gridUnit
                bottomMargin: Kirigami.Units.gridUnit
                rightMargin: Kirigami.Units.gridUnit
            }

            ActionButton {
                id: rotateButton
                Layout.alignment: Qt.AlignCenter

                icon.name: "object-rotate-left-symbolic"
                toolTip: !rotateHandle.pressed ? i18nc("@action:button tooltip rotate widget", "Click and drag to rotate") : ""
                action: applet ? applet.plasmoid.internalAction("rotate") : null
                down: rotateHandle.pressed
                Component.onCompleted: {
                    if (action !== null) {
                        action.enabled = true;
                    }
                }
                MouseArea {
                    id: rotateHandle
                    anchors.fill: parent

                    property int startRotation
                    property real startCenterRelativeAngle

                    function pointAngle(pos: point): real {
                        var r = Math.sqrt(pos.x * pos.x + pos.y * pos.y);
                        var cosine = pos.x / r;

                        if (pos.y >= 0) {
                            return Math.acos(cosine) * (180/Math.PI);
                        } else {
                            return -Math.acos(cosine) * (180/Math.PI);
                        }
                    }

                    function centerRelativePos(x: real, y: real): point {
                        var mousePos = overlay.itemContainer.parent.mapFromItem(rotateButton, x, y);
                        var centerPos = overlay.itemContainer.parent.mapFromItem(overlay.itemContainer, overlay.itemContainer.width/2, overlay.itemContainer.height/2);

                        mousePos.x -= centerPos.x;
                        mousePos.y -= centerPos.y;
                        return mousePos;
                    }

                    onPressed: mouse => {
                        mouse.accepted = true;
                        startRotation = overlay.itemContainer.rotation;
                        startCenterRelativeAngle = pointAngle(centerRelativePos(mouse.x, mouse.y));
                    }

                    onPositionChanged: mouse => {
                        var rot = startRotation % 360;
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

                        overlay.itemContainer.rotation = newRotation;
                    }

                    onReleased: mouse => {
                        // save rotation
                        overlay.itemContainer.layout.save();
                    }
                }
            }
        }
    }
    
}