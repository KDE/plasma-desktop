/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

MouseArea {
    id: configurationArea

    z: 1000

    anchors.fill: currentLayout

    hoverEnabled: true

    property bool isResizingLeft: false
    property bool isResizingRight: false
    property Item currentApplet

    property int lastX
    property int lastY

    readonly property int spacerHandleSize: PlasmaCore.Units.smallSpacing

    onHeightChanged: tooltip.visible = false;
    onWidthChanged: tooltip.visible = false;

    onPositionChanged: {
        if (currentApplet && currentApplet.applet &&
            currentApplet.applet.pluginName === "org.kde.plasma.panelspacer") {
            if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                if ((mouse.y - handle.y) < spacerHandleSize ||
                    (mouse.y - handle.y) > (handle.height - spacerHandleSize)) {
                    configurationArea.cursorShape = Qt.SizeVerCursor;
                } else {
                    configurationArea.cursorShape = Qt.ArrowCursor;
                }
            } else {
                if ((mouse.x - handle.x) < spacerHandleSize ||
                    (mouse.x - handle.x) > (handle.width - spacerHandleSize)) {
                    configurationArea.cursorShape = Qt.SizeHorCursor;
                } else {
                    configurationArea.cursorShape = Qt.ArrowCursor;
                }
            }
        } else {
            if (configurationArea.containsPress) {
                configurationArea.cursorShape = Qt.ClosedHandCursor;
            } else {
                configurationArea.cursorShape = Qt.OpenHandCursor;
            }
        }

        if (pressed) {
            if (currentApplet && currentApplet.applet.pluginName === "org.kde.plasma.panelspacer") {

                if (isResizingLeft) {
                    if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                        handle.y += (mouse.y - lastY);
                        handle.height = currentApplet.height + (currentApplet.y - handle.y);
                    } else {
                        handle.x += (mouse.x - lastX);
                        handle.width = currentApplet.width + (currentApplet.x - handle.x);
                    }

                    lastX = mouse.x;
                    lastY = mouse.y;
                    return;

                } else if (isResizingRight) {
                    if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                        handle.height = mouse.y - handle.y;
                    } else {
                        handle.width = mouse.x - handle.x;
                    }

                    lastX = mouse.x;
                    lastY = mouse.y;
                    return;
                }
            }

            var padding = PlasmaCore.Units.gridUnit * 3;
            if (currentApplet && (mouse.x < -padding || mouse.y < -padding ||
                mouse.x > width + padding || mouse.y > height + padding)) {
                var newCont = Plasmoid.containmentAt(mouse.x, mouse.y);

                if (newCont && newCont !== plasmoid) {
                    var newPos = newCont.mapFromApplet(plasmoid, mouse.x, mouse.y);
                    var applet = currentApplet.applet;
                    currentApplet.destroy();
                    applet.anchors.fill = undefined
                    newCont.addApplet(applet, newPos.x, newPos.y);
                    return;
                }
            }

            if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                currentApplet.y += (mouse.y - lastY);
                handle.y = currentApplet.y;
            } else {
                currentApplet.x += (mouse.x - lastX);
                handle.x = currentApplet.x;
            }

            lastX = mouse.x;
            lastY = mouse.y;

            var item = currentLayout.childAt(mouse.x, mouse.y);

            if (item && item !== placeHolder) {
                placeHolder.width = item.width;
                placeHolder.height = item.height;
                placeHolder.parent = configurationArea;
                var posInItem = mapToItem(item, mouse.x, mouse.y);
                let i = 0;

                if ((Plasmoid.formFactor === PlasmaCore.Types.Vertical && posInItem.y < item.height/2) ||
                    (Plasmoid.formFactor !== PlasmaCore.Types.Vertical && posInItem.x < item.width/2)) {
                    i = root.layoutManager.insertBefore(item, placeHolder);
                } else {
                    i = root.layoutManager.insertAfter(item, placeHolder);
                }
                if (i!=undefined) {root.layoutManager.updateMargins()}
            }

        } else {
            var item = currentLayout.childAt(mouse.x, mouse.y);
            if (root.dragOverlay && item && item !== lastSpacer) {
                root.dragOverlay.currentApplet = item;
            } else {
                root.dragOverlay.currentApplet = null;
            }
        }

        if (root.dragOverlay.currentApplet) {
            hideTimer.stop();
            tooltip.visible = true;
            tooltip.raise();
        }
    }

    onEntered: hideTimer.stop();

    onCurrentAppletChanged: {
        if (!currentApplet || !root.dragOverlay.currentApplet) {
            hideTimer.start();
            return;
        }
        handle.x = currentApplet.x;
        handle.y = currentApplet.y;
        handle.width = currentApplet.width;
        handle.height = currentApplet.height;
    }

    onPressed: {
        // Need to set currentApplet here too, to make touch selection + drag
        // with with a touchscreen, because there are no entered events in that
        // case
        let item = currentLayout.childAt(mouse.x, mouse.y);
        if (item) {
            currentApplet = item;
            root.dragOverlay.currentApplet = item;
            tooltip.visible = true;
            tooltip.raise();
            hideTimer.stop();
        }

        if (!root.dragOverlay.currentApplet) {
            return;
        }

        if (currentApplet.applet.pluginName === "org.kde.plasma.panelspacer") {
            if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                if ((mouse.y - handle.y) < spacerHandleSize) {
                    configurationArea.isResizingLeft = true;
                    configurationArea.isResizingRight = false;
                } else if ((mouse.y - handle.y) > (handle.height - spacerHandleSize)) {
                    configurationArea.isResizingLeft = false;
                    configurationArea.isResizingRight = true;
                } else {
                    configurationArea.isResizingLeft = false;
                    configurationArea.isResizingRight = false;
                }

            } else {
                if ((mouse.x - handle.x) < spacerHandleSize) {
                    configurationArea.isResizingLeft = true;
                    configurationArea.isResizingRight = false;
                } else if ((mouse.x - handle.x) > (handle.width - spacerHandleSize)) {
                    configurationArea.isResizingLeft = false;
                    configurationArea.isResizingRight = true;
                } else {
                    configurationArea.isResizingLeft = false;
                    configurationArea.isResizingRight = false;
                }
            }
        }

        lastX = mouse.x;
        lastY = mouse.y;
        placeHolder.width = currentApplet.width;
        placeHolder.height = currentApplet.height;
        placeHolder.dragging = currentApplet;
        root.layoutManager.insertBefore(currentApplet, placeHolder);
        currentApplet.parent = moveAppletLayer;
        currentApplet.z = 900;
    }

    onReleased: finishDragOperation()

    onCanceled: finishDragOperation()

    function finishDragOperation() {
        if (!currentApplet) {
            return;
        }

        if (Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
            currentApplet.applet.configuration.length = handle.height;
        } else {
            currentApplet.applet.configuration.length = handle.width;
        }

        configurationArea.isResizingLeft = false;
        configurationArea.isResizingRight = false;

        root.layoutManager.insertBefore(placeHolder, currentApplet);
        placeHolder.parent = configurationArea;
        currentApplet.z = 1;

        handle.x = currentApplet.x;
        handle.y = currentApplet.y;
        handle.width = currentApplet.width;
        handle.height = currentApplet.height;
        root.layoutManager.save();
    }

    Item {
        id: placeHolder
        property Item dragging
        visible: configurationArea.containsMouse
        Layout.fillWidth: currentApplet ? currentApplet.Layout.fillWidth : false
        Layout.fillHeight: currentApplet ? currentApplet.Layout.fillHeight : false
    }

    Timer {
        id: hideTimer
        interval: PlasmaCore.Units.longDuration
        onTriggered: tooltip.visible = false;
    }

    Connections {
        target: currentApplet
        function onXChanged() {handle.x = currentApplet.x}
        function onYChanged() {handle.y = currentApplet.y}
        function onWidthChanged() {handle.width = currentApplet.width}
        function onHeightChanged() {handle.height = currentApplet.height}
    }

    Rectangle {
        id: handle
        visible: configurationArea.containsMouse
        color: PlasmaCore.Theme.backgroundColor
        radius: 3
        opacity: currentApplet ? 0.5 : 0
        PlasmaCore.IconItem {
            source: "transform-move"
            width: Math.min(parent.width, parent.height)
            height: width
            anchors.centerIn: parent
        }
        Rectangle {
            anchors {
                left: parent.left
                top: parent.top
                bottom: (Plasmoid.formFactor !== PlasmaCore.Types.Vertical) ? parent.bottom : undefined
                right: (Plasmoid.formFactor !== PlasmaCore.Types.Vertical) ? undefined : parent.right
            }
            visible: currentApplet && currentApplet.applet.pluginName === "org.kde.plasma.panelspacer"
            opacity: visible && !xAnim.running && !yAnim.running ? 1.0 : 0
            width: configurationArea.spacerHandleSize
            height: configurationArea.spacerHandleSize
            color: PlasmaCore.Theme.textColor
            Behavior on opacity {
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
        Rectangle {
            anchors {
                right: parent.right
                top: (Plasmoid.formFactor !== PlasmaCore.Types.Vertical) ? parent.top : undefined
                bottom: parent.bottom
                left: (Plasmoid.formFactor !== PlasmaCore.Types.Vertical) ? undefined : parent.left
            }
            visible: currentApplet && currentApplet.applet.pluginName === "org.kde.plasma.panelspacer"
            opacity: visible && !xAnim.running && !yAnim.running ? 1.0 : 0
            width: configurationArea.spacerHandleSize
            height: configurationArea.spacerHandleSize
            color: PlasmaCore.Theme.textColor
            Behavior on opacity {
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
        Behavior on x {
            enabled: !configurationArea.pressed
            NumberAnimation {
                id: xAnim
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on y {
            id: yAnim
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on width {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on height {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
    PlasmaCore.Dialog {
        id: tooltip
        visualParent: currentApplet

        type: PlasmaCore.Dialog.Dock
        flags: Qt.WindowStaysOnTopHint|Qt.WindowDoesNotAcceptFocus|Qt.BypassWindowManagerHint
        location: Plasmoid.location

        onVisualParentChanged: {
            if (visualParent) {
                currentApplet.applet.prepareContextualActions();
                alternativesButton.visible = currentApplet.applet.action("alternatives") && currentApplet.applet.action("alternatives").enabled;
                configureButton.visible = currentApplet.applet.action("configure") && currentApplet.applet.action("configure").enabled;
                label.text = currentApplet.applet.title;
            }
        }

        mainItem: MouseArea {
            enabled: currentApplet
            width: handleButtons.width
            height: handleButtons.height
            hoverEnabled: true
            onEntered: hideTimer.stop();
            onExited:  hideTimer.restart();

            ColumnLayout {
                id: handleButtons
                spacing: PlasmaCore.Units.smallSpacing

                PlasmaExtras.PlasmoidHeading {
                    leftPadding: PlasmaCore.Units.smallSpacing * 2
                    rightPadding: PlasmaCore.Units.smallSpacing * 2

                    contentItem: PlasmaExtras.Heading {
                        id: label
                        level: 3
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                PlasmaComponents.ToolButton {
                    Layout.fillWidth: true
                    // we want destructive actions to be far from the initial
                    // cursor position, so show this on the top unless it's on
                    // a top panel
                    visible: tooltip.location !== PlasmaCore.Types.TopEdge
                             && currentApplet
                             && currentApplet.applet.action("remove")
                             && currentApplet.applet.action("remove").enabled
                    iconSource: "delete"
                    text: i18n("Remove")
                    onClicked: {
                        tooltip.visible = false;
                        currentApplet.applet.action("remove").trigger();
                    }
                }
                PlasmaComponents.ToolButton {
                    id: configureButton
                    Layout.fillWidth: true
                    iconSource: "configure"
                    text: i18n("Configure…")
                    onClicked: {
                        tooltip.visible = false;
                        currentApplet.applet.action("configure").trigger()
                    }
                }
                PlasmaComponents.ToolButton {
                    id: alternativesButton
                    Layout.fillWidth: true
                    iconSource: "widget-alternatives"
                    text: i18n("Show Alternatives…")
                    onClicked: {
                        tooltip.visible = false;
                        currentApplet.applet.action("alternatives").trigger()
                    }
                }
                PlasmaComponents.ToolButton {
                    Layout.fillWidth: true
                    // we want destructive actions to be far from the initial
                    // cursor position, so show this on the bottom for top panels
                    visible: tooltip.location === PlasmaCore.Types.TopEdge
                             && currentApplet
                             && currentApplet.applet.action("remove")
                             && currentApplet.applet.action("remove").enabled
                    iconSource: "delete"
                    text: i18n("Remove")
                    onClicked: {
                        tooltip.visible = false;
                        currentApplet.applet.action("remove").trigger();
                    }
                }
            }
        }
    }
}
