/*
 *   Copyright 2011-2013 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *   Copyright 2014 David Edmundson <davidedmundson@kde.org>
 *   Copyright 2015 Eike Hein <hein@kde.org>
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
import org.kde.plasma.plasmoid 2.0

Item {
    id: appletItem

    property int handleWidth: iconSize
    property int minimumHandleHeight: 7 * (root.iconSize + 7) + margins.top + margins.bottom
    property int handleHeight: (height < minimumHandleHeight) ? minimumHandleHeight : height
    property string category

    property bool showAppletHandle: temporaryShowAppletHandle || plasmoid.editMode
    property bool temporaryShowAppletHandle: false
    property real controlsOpacity: (plasmoid.immutable || !showAppletHandle) ? 0 : 1
    property string backgroundHints: "NoBackground"
    property bool hasBackground: false
    property bool handleMerged: (height > minimumHandleHeight && !appletHandle.forceFloating)
    property bool animationsEnabled: false
    property bool floating: true // turns off layoutManagment space handling for appletItem

    property alias innerEndHeight: mouseListener.endHeight
    property alias innerEndWidth: mouseListener.endWidth
    property alias innerHeight: mouseListener.height
    property alias innerWidth: mouseListener.width

    property int minimumWidth: Math.max(root.layoutManager.cellSize.width,
                           appletContainer.minimumWidth +
                           margins.left +
                           margins.right);

    property int minimumHeight: Math.max(root.layoutManager.cellSize.height,
                            appletContainer.minimumHeight +
                            margins.top +
                            margins.bottom);

    property int maximumWidth: appletContainer.maximumWidth +
                               margins.left +
                               margins.right;

    property int maximumHeight: appletContainer.maximumHeight +
                                margins.top +
                                margins.bottom;

    property alias applet: appletContainer.applet

    property Item contents: appletContainer
    property alias margins: plasmoidBackground.margins
    property alias imagePath: plasmoidBackground.imagePath

    visible: false

    QtObject {
        id: d

        property real lastX: 0
        property real lastY: 0
    }

    Timer {
        id: positionTimer

        repeat: false
        running: false
        interval: 100

        onTriggered: {
            releasePosition();
            positionItem();
            if (showAppletHandle && !handleMerged)
                appletHandle.positionHandle();
        }
    }

    onMinimumWidthChanged: {
        if (width < minimumWidth) {
            width = minimumWidth;
            positionTimer.restart();
        }
    }
    onMinimumHeightChanged: {
        if (height < minimumHeight) {
            height = minimumHeight;
            positionTimer.restart();
        }
    }
    onMaximumWidthChanged: {
        if (width > maximumWidth) {
            width = maximumWidth;
            positionTimer.restart();
        }
    }
    onMaximumHeightChanged: {
        if (height > maximumHeight) {
            height = maximumHeight;
            positionTimer.restart();
        }
    }

    onHeightChanged: {
        if (height > maximumHeight)
            innerEndHeight = maximumHeight;
        else if (height < minimumHeight)
            innerEndHeight = minimumHeight;
        else
            innerEndHeight = height;
    }
    onWidthChanged: {
        if (width > maximumWidth)
            innerEndWidth = maximumWidth;
        else if (width < minimumWidth)
            innerEndWidth =  minimumWidth;
        else
            innerEndWidth = width;
    }

    onXChanged: {
        if (animationsEnabled) {
            animationsEnabled = false;
            mouseListener.x += d.lastX - x;
            animationsEnabled = true;
        }
        mouseListener.x = mouseListener.endX = (width - innerEndWidth)/2;
        d.lastX = x;
    }
    onYChanged: {
        if (animationsEnabled) {
            animationsEnabled = false;
            mouseListener.y += d.lastY - y;
            animationsEnabled = true;
        }
        mouseListener.y = mouseListener.endY = (height - innerEndHeight)/2;
        d.lastY = y;
    }

    // use this function to position appletItem instead of root.layoutManager.positionItem(appletItem)
    function positionItem() {
        if (floating)
            return;
        root.layoutManager.positionItem(appletItem);
    }

    // use this function to free appletItem position instead of
    // root.layoutManager.setSpaceAvailable(appletItem.x, appletItem.y, appletItem.width, appletItem.height, true)
    function releasePosition() {
        if (!floating)
            root.layoutManager.setSpaceAvailable(x, y, width, height, true);
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

    KQuickControlsAddons.MouseEventListener {
        id: mouseListener
        // used as inner applet (which would not be sized over or under size limits )
        // centered in appletItem.onXChanged and onYChanged due to allow animations on X and Y

        property int pressX: -1
        property int pressY: -1

        // for animations
        property int endHeight: minimumHeight
        property int endWidth: minimumWidth
        property int endX: 0
        property int endY: 0

        function startDrag(mouse) {
            dragMouseArea.dragging = true;

            eventGenerator.sendGrabEventRecursive(appletItem, KQuickControlsAddons.EventGenerator.UngrabMouse);
            eventGenerator.sendGrabEvent(dragMouseArea, KQuickControlsAddons.EventGenerator.GrabMouse);
            eventGenerator.sendMouseEvent(dragMouseArea, KQuickControlsAddons.EventGenerator.MouseButtonPress, mouse.x, mouse.y, Qt.LeftButton, Qt.LeftButton, 0);
        }

        height: endHeight
        width: endWidth
        z: 10

        hoverEnabled: true

        onPressed: {
            if (!plasmoid.immutable && mouse.modifiers == Qt.AltModifier) {
                if (!dragMouseArea.dragging) {
                    startDrag(mouse)
                }

                return;
            }

            pressX = mouse.x;
            pressY = mouse.y;
        }

        onPressAndHold: {
            if (!plasmoid.immutable && plasmoid.configuration.pressToMove) {
                if (!dragMouseArea.dragging && !root.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                    temporaryShowAppletHandle = true;

                    startDrag(mouse)
                }
            }

            pressX = -1;
            pressY = -1;
        }

        onContainsMouseChanged: {
            animationsEnabled = true;

            if (!plasmoid.immutable && (!plasmoid.configuration.pressToMove || !containsMouse)) {
                hoverTracker.restart();
            }
        }

        Timer {
            id: hoverTracker
            repeat: false
            interval: root.handleDelay
            onTriggered: {
                if (mouseListener.containsMouse || (appletHandle.item && (appletHandle.item.containsMouse || appletHandle.item.pressed))) {
                    if (!plasmoid.configuration.pressToMove) {
                        temporaryShowAppletHandle = true;
                    }
                } else if (!dragMouseArea.dragging) {
                    temporaryShowAppletHandle = false;
                }
            }
        }

        Item {
            anchors { left: parent.left; top: parent.top; bottom: parent.bottom; }
            width: parent.width+handleWidth;
            z: mouseListener.z + 4

            PlasmaCore.FrameSvgItem {
                id: plasmoidBackground
                visible: backgroundHints != PlasmaCore.Types.NoBackground
                imagePath: "widgets/background"
                anchors { left: parent.left; top: parent.top; bottom: parent.bottom; }
                width: parent.width - _handleWidth
                smooth: true

                property real _handleWidth: (showAppletHandle && handleMerged) ? 0 : handleWidth

                Behavior on _handleWidth {
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
                    temporaryShowAppletHandle = false;
                }
                onAppletRemoved: {
//                     print("Applet removed Applet-" + applet.id)
                    if (applet.id == appletItem.applet.id) {
//                         print("Destroying Applet-" + applet.id)
                        root.layoutManager.saveRotation(appletItem);
                        appletItem.releasePosition();
                        //applet.action("remove").trigger();
                        //appletItem.destroy()
                        appletItem.destroy();
                    }
                }
            }
            Connections {
                target: applet

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
                property bool dragging: false // Set by mouseListener.onPressAndHold -- drag.active only becomes true on movement.

                function endDrag() {
                    appletItem.z = appletItem.z - zoffset;
                    repositionTimer.running = false;
                    placeHolderPaint.opacity = 0;
                    animationsEnabled = true;
                    appletItem.floating = false;
                    appletItem.positionItem();
                    root.layoutManager.save();
                    dragging = false;
                }

                onDraggingChanged: {
                    cursorShape = dragging ? Qt.DragMoveCursor : Qt.ArrowCursor;
                }

                onPressed: {
                    appletItem.z = appletItem.z + zoffset;
                    animationsEnabled = false;
                    mouse.accepted = true;
                    appletItem.releasePosition();
                    appletItem.floating = true;

                    placeHolder.syncWithItem(appletItem);
                    placeHolderPaint.opacity = root.haloOpacity;
                }

                onPositionChanged: {
                    var pos = mapToItem(root.parent, mouse.x, mouse.y);
                    var newCont = plasmoid.containmentAt(pos.x, pos.y);

                    if (newCont && newCont != plasmoid) {
                        var newPos = newCont.mapFromApplet(plasmoid, pos.x, pos.y);
                        newCont.addApplet(appletItem.applet, newPos.x, newPos.y);
                        placeHolderPaint.opacity = 0;
                    } else {
                        placeHolder.syncWithItem(appletItem);
                    }
                }

                onCanceled: {
                    endDrag();
                    appletHandle.positionHandle();
                }

                onReleased: {
                    endDrag();
                    appletHandle.positionHandle();
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
                readonly property int maxInt: 1000000 // dirty hack to convert js number to qml int

                property var minimumSize: {
                    var size;
                    if (applet && applet.Layout) {
                        var layout = applet.Layout
                        size = { 'width': layout.minimumWidth,
                                 'height': layout.minimumHeight };
                    } else {
                        size = { 'width': 0, 'height': 0 };
                    }

                    return size;
                }

                property var maximumSize: {
                    var size;
                    if (applet && applet.Layout) {
                        var layout = applet.Layout
                        size = { 'width': layout.maximumWidth,
                                 'height': layout.maximumHeight };
                    } else {
                        size = { 'width': Number.POSITIVE_INFINITY,
                                 'height': Number.POSITIVE_INFINITY };

                    }

                    if (size.width > maxInt)
                        size.width = maxInt;
                    if (size.height > maxInt)
                        size.height = maxInt;

                    return size;
                }

                onMinimumSizeChanged: {
                    minimumHeight = minimumSize.height
                    if (minimumHeight > maximumSize.height)
                        maximumHeight = minimumHeight
                    else if (maximumHeight !== maximumSize.height)
                        maximumHeight = maximumSize.height

                    minimumWidth = minimumSize.width
                    if (minimumWidth > maximumSize.width)
                        maximumWidth = minimumWidth
                    else if (maximumWidth !== maximumSize.width)
                        maximumWidth = maximumSize.width
                }

                onMaximumSizeChanged: {
                    maximumHeight = maximumSize.height
                    if (maximumHeight < minimumSize.height)
                        minimumHeight = maximumHeight
                    else if (minimumHeight !== minimumSize.height)
                        minimumHeight = minimumSize.height

                    maximumWidth = maximumSize.width
                    if (maximumWidth < minimumSize.width)
                        minimumWidth = maximumWidth
                    else if (minimumWidth !== minimumSize.width)
                        minimumWidth = minimumSize.width
                }

                property int minimumWidth: 0
                property int minimumHeight: 0

                property int maximumWidth: maxInt
                property int maximumHeight: maxInt

                function appletDestroyed() {
//                     print("Applet DESTROYED.");
                    appletItem.releasePosition();
                    applet.action("remove").trigger();
                    appletItem.destroy()
                }

                onAppletChanged: {
                    if (!applet) {
                        return;
                    }
                    applet.parent = appletContainer;
                    applet.anchors.fill = appletContainer;

                    updateBackgroundHints();
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
                        script: {
                            appletItem.scale = 1;
                            appletContainer.appletDestroyed();
                        }
                    }
                }
                Loader {
                    id: busyLoader

                    anchors.centerIn: parent

                    z: appletContainer.z + 1

                    active: applet && applet.busy

                    source: "BusyOverlay.qml"
                    asynchronous: true
                }

                Component.onCompleted: PlasmaExtras.AppearAnimation {
                    targetItem: appletItem
                }
            }

            Loader {
                id: appletHandle
                z: appletContainer.z + 1
                property bool forceFloating : false
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: plasmoidBackground.right
                }
                Connections {
                    target: appletItem
                    onShowAppletHandleChanged: {
                        if (appletItem.showAppletHandle) {
                            appletItem.z += dragMouseArea.zoffset;
                            appletHandle.positionHandle();
                            if (appletHandle.source == "") {
                                appletHandle.source = "AppletHandle.qml";
                            }
                        } else {
                            appletItem.z -= dragMouseArea.zoffset;
                        }
                    }
                    onHandleMergedChanged: {
                        if (appletItem.handleMerged) {
                            appletHandle.anchors.verticalCenterOffset = 0;
                        } else {
                            appletHandle.positionHandle();
                        }
                    }
                }
                Connections {
                    target: appletHandle.item
                    onMoveFinished: {
                        appletHandle.positionHandle();
                    }
                }

                function positionHandle()
                {
                    // Don't show handle outside of desktop
                    var available = plasmoid.availableScreenRect;
                    var x = Math.min(Math.max(0, appletItem.x + mouseListener.endX), available.width - appletItem.width + mouseListener.endX);
                    var y = Math.min(Math.max(0, appletItem.y + mouseListener.endY), available.height - appletItem.height + mouseListener.endY);
                    var verticalCenter = (y + mouseListener.endHeight / 2);
                    var topOutside = (verticalCenter - minimumHandleHeight / 2);
                    var bottomOutside = verticalCenter + minimumHandleHeight / 2 - available.height;
                    if (bottomOutside > 0) {
                        anchors.verticalCenterOffset = -bottomOutside;
                    } else if (topOutside < 0) {
                        anchors.verticalCenterOffset = -topOutside;
                    } else {
                        anchors.verticalCenterOffset = 0;
                    }
                    var rightOutside = x + mouseListener.endWidth + handleWidth - available.width;
                    appletHandle.anchors.rightMargin = appletItem.margins.right + Math.max(0, rightOutside);
                    appletHandle.forceFloating = rightOutside > 0;
                }
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
    }

    Behavior on controlsOpacity {
        NumberAnimation {
            duration: units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    //sides
    ResizeHandle {
        anchors {
            left: parent.left
            top: parent.top
            right: parent.right
            leftMargin: units.gridUnit
            rightMargin: units.gridUnit
        }
        cursorShape: Qt.SizeVerCursor
        moveX: false
        moveY: true
        resizeWidth: false
        resizeHeight: true
    }
    ResizeHandle {
        anchors {
            left: parent.left
            bottom: parent.bottom
            right: parent.right
            leftMargin: units.gridUnit
            rightMargin: units.gridUnit
        }
        cursorShape: Qt.SizeVerCursor
        moveX: false
        moveY: false
        resizeWidth: false
        resizeHeight: true
    }
    ResizeHandle {
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            topMargin: units.gridUnit
            bottomMargin: units.gridUnit
        }
        enabled: !parent.LayoutMirroring.enabled || !showAppletHandle
        cursorShape: Qt.SizeHorCursor
        moveX: true
        moveY: false
        resizeWidth: true
        resizeHeight: false
    }
    ResizeHandle {
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            topMargin: units.gridUnit
            bottomMargin: units.gridUnit
        }
        enabled: parent.LayoutMirroring.enabled || !showAppletHandle
        cursorShape: Qt.SizeHorCursor
        moveX: false
        moveY: false
        resizeWidth: true
        resizeHeight: false
    }
    //corners
    ResizeHandle {
        anchors {
            left: parent.left
            top: parent.top
        }
        enabled: !parent.LayoutMirroring.enabled || !showAppletHandle
        cursorShape: Qt.SizeFDiagCursor
        moveX: true
        moveY: true
        resizeWidth: true
        resizeHeight: true
    }
    ResizeHandle {
        anchors {
            right: parent.right
            top: parent.top
        }
        enabled: parent.LayoutMirroring.enabled || !showAppletHandle
        cursorShape: Qt.SizeBDiagCursor
        moveX: false
        moveY: true
        resizeWidth: true
        resizeHeight: true
    }
    ResizeHandle {
        anchors {
            left: parent.left
            bottom: parent.bottom
        }
        enabled: !parent.LayoutMirroring.enabled || !showAppletHandle
        cursorShape: Qt.SizeBDiagCursor
        moveX: true
        moveY: false
        resizeWidth: true
        resizeHeight: true
    }
    ResizeHandle {
        anchors {
            right: parent.right
            bottom: parent.bottom
        }
        enabled: parent.LayoutMirroring.enabled || !showAppletHandle
        cursorShape: Qt.SizeFDiagCursor
        moveX: false
        moveY: false
        resizeWidth: true
        resizeHeight: true
    }

    Component.onCompleted: {
        floating = false;
        layoutTimer.running = true
        layoutTimer.restart()
        visible = false
    }
}
