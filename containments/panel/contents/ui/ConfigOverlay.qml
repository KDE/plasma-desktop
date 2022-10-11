/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kirigami 2.20 as Kirigami

MouseArea {
    id: configurationArea

    z: 1000
    hoverEnabled: true

    property Item currentApplet
    property real startDragOffset: 0.0

    onPositionChanged: {
        if (pressed) {

            // If the object has been dragged outside of the panel and there's
            // a different containment there, we remove it from the panel
            // containment and add it to the new one.
            var padding = PlasmaCore.Units.gridUnit * 5;
            if (currentApplet && (mouse.x < -padding || mouse.y < -padding ||
                mouse.x > width + padding || mouse.y > height + padding)) {
                var newCont = plasmoid.containmentAt(mouse.x, mouse.y);

                if (newCont && newCont !== plasmoid) {
                    var newPos = newCont.mapFromApplet(plasmoid, mouse.x, mouse.y);
                    var applet = currentApplet.applet;
                    appletsModel.remove(placeHolder.parent.index);
                    currentApplet.destroy();
                    applet.anchors.fill = undefined
                    newCont.addApplet(applet, newPos.x, newPos.y);
                    return;
                }
            }
            if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                currentApplet.y = mouse.y - configurationArea.startDragOffset;
            } else {
                currentApplet.x = mouse.x - configurationArea.startDragOffset;
            }

            var item = root.layoutManager.childAtCoordinates(mouse.x, mouse.y);

            if (item && item.applet !== placeHolder) {
                var posInItem = mapToItem(item, mouse.x, mouse.y)
                var pos = root.isHorizontal ? posInItem.x : posInItem.y
                var size = root.isHorizontal ? item.width : item.height
                if (pos < size / 3) {
                    root.layoutManager.move(placeHolder.parent, item.index)
                } else if (pos > size / 3 * 2) {
                    root.layoutManager.move(placeHolder.parent, item.index+1)
                }
            }

        } else {
            var item = currentLayout.childAt(mouse.x, mouse.y);
            if (configurationArea && item && item !== lastSpacer) {
                configurationArea.currentApplet = item;
            }
        }

        if (configurationArea.currentApplet) {
            hideTimer.stop();
            tooltip.raise();
        }
    }

    onEntered: hideTimer.stop();

    onExited: hideTimer.restart()

    onCurrentAppletChanged: {
        if (!currentApplet || !configurationArea.currentApplet) {
            hideTimer.start();
            return;
        }
    }

    onPressed: {
        // Need to set currentApplet here too, to make touch selection + drag
        // with with a touchscreen, because there are no entered events in that
        // case
        let item = currentLayout.childAt(mouse.x, mouse.y);
        // BUG 454095: Don't allow dragging lastSpacer as it's not a real applet
        if (!item || item == lastSpacer) {
            return;
        }
        tooltip.raise();
        hideTimer.stop();

        // We set the current applet being dragged as a property of placeHolder
        // to be able to read its properties from the LayoutManager
        appletsModel.insert(item.index, {applet: placeHolder});
        placeHolder.parent.inThickArea = item.inThickArea
        currentApplet = appletContainerComponent.createObject(root, {applet: item.applet, x: item.x, y: item.y, z: 900,
                                                                     width: item.width, height: item.height, index: -1})
        placeHolder.parent.dragging = currentApplet
        appletsModel.remove(item.index)
        root.dragAndDropping = true

        if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
            configurationArea.startDragOffset = mouse.y - currentApplet.y;
        } else {
            configurationArea.startDragOffset = mouse.x - currentApplet.x;
        }
    }

    onReleased: finishDragOperation()

    onCanceled: finishDragOperation()

    function finishDragOperation() {
        root.dragAndDropping = false
        if (!currentApplet) {
            return;
        }
        appletsModel.set(placeHolder.parent.index, {applet: currentApplet.applet})
        let newCurrentApplet = currentApplet.applet.parent
        newCurrentApplet.animateFrom(currentApplet.x, currentApplet.y)
        newCurrentApplet.dragging = null
        placeHolder.parent = configurationArea;
        currentApplet.destroy()
        root.layoutManager.save()
    }

    Item {
        id: placeHolder
        property Item dragging
        property bool busy: false
        visible: configurationArea.containsMouse
        Layout.preferredWidth: currentApplet ? currentApplet.Layout.preferredWidth : 0
        Layout.preferredHeight: currentApplet ? currentApplet.Layout.preferredHeight : 0
        Layout.maximumWidth: currentApplet ? currentApplet.Layout.maximumWidth : 0
        Layout.maximumHeight: currentApplet ? currentApplet.Layout.maximumHeight : 0
        Layout.minimumWidth: currentApplet ? currentApplet.Layout.minimumWidth : 0
        Layout.minimumHeight: currentApplet ? currentApplet.Layout.minimumHeight : 0
        Layout.fillWidth: currentApplet ? currentApplet.Layout.fillWidth : false
        Layout.fillHeight: currentApplet ? currentApplet.Layout.fillHeight : false
    }

    Timer {
        id: hideTimer
        interval: PlasmaCore.Units.longDuration * 5
        onTriggered: currentApplet = null
    }

    Rectangle {
        id: handle
        x: currentApplet ? currentApplet.x : NaN
        y: currentApplet ? currentApplet.y : NaN
        width: currentApplet ? currentApplet.width : NaN
        height: currentApplet ? currentApplet.height : NaN
        color: PlasmaCore.Theme.backgroundColor
        radius: 3
        opacity: currentApplet && configurationArea.containsMouse ? 0.5 : 0
        PlasmaCore.IconItem {
            visible: !root.dragAndDropping
            source: "transform-move"
            width: Math.min(parent.width, parent.height)
            height: width
            anchors.centerIn: parent
        }
        Behavior on x {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on y {
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
        visible: currentApplet && !root.dragAndDropping
        visualParent: currentApplet

        type: PlasmaCore.Dialog.Dock
        flags: Qt.WindowStaysOnTopHint|Qt.WindowDoesNotAcceptFocus|Qt.BypassWindowManagerHint
        location: plasmoid.location

        onVisualParentChanged: {
            if (visualParent) {
                currentApplet.applet.prepareContextualActions();
                alternativesButton.visible = currentApplet.applet.action("alternatives") && currentApplet.applet.action("alternatives").enabled;
                configureButton.visible = currentApplet.applet.action("configure") && currentApplet.applet.action("configure").enabled;
                label.text = currentApplet.applet.title;
            }
        }

        mainItem: MouseArea {
            enabled: tooltip.visible
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

                PlasmaComponents3.ToolButton {
                    Layout.fillWidth: true
                    // we want destructive actions to be far from the initial
                    // cursor position, so show this on the top unless it's on
                    // a top panel
                    visible: tooltip.location !== PlasmaCore.Types.TopEdge
                             && currentApplet
                             && currentApplet.applet.action("remove")
                             && currentApplet.applet.action("remove").enabled
                    icon.name: "delete"
                    text: i18n("Remove")
                    onClicked: {
                        currentApplet.applet.action("remove").trigger();
                        currentApplet = null
                    }
                }
                PlasmaComponents3.ToolButton {
                    id: configureButton
                    Layout.fillWidth: true
                    icon.name: "configure"
                    text: i18n("Configure…")
                    onClicked: {
                        currentApplet.applet.action("configure").trigger()
                        currentApplet = null
                    }
                }
                PlasmaComponents3.ToolButton {
                    id: alternativesButton
                    Layout.fillWidth: true
                    icon.name: "widget-alternatives"
                    text: i18n("Show Alternatives…")
                    onClicked: {
                        currentApplet.applet.action("alternatives").trigger()
                        currentApplet = null
                    }
                }
                PlasmaComponents3.ToolButton {
                    Layout.fillWidth: true
                    // we want destructive actions to be far from the initial
                    // cursor position, so show this on the bottom for top panels
                    visible: tooltip.location === PlasmaCore.Types.TopEdge
                             && currentApplet
                             && currentApplet.applet.action("remove")
                             && currentApplet.applet.action("remove").enabled
                    icon.name: "delete"
                    text: i18n("Remove")
                    onClicked: {
                        currentApplet.applet.action("remove").trigger()
                        currentApplet = null
                    }
                }

                PlasmaExtras.Heading {
                    Layout.fillWidth: true
                    visible: panelSpacerWidth.visible
                    text: i18n("Spacer width")
                    level: 3
                    horizontalAlignment: Text.AlignHCenter
                }

                PlasmaComponents3.SpinBox {
                    id: panelSpacerWidth
                    editable: true
                    Layout.fillWidth: true
                    focus: !Kirigami.InputMethod.willShowOnActive
                    visible: currentApplet && currentApplet.applet.pluginName === "org.kde.plasma.panelspacer" && !currentApplet.applet.configuration.expanding
                    from: 0
                    stepSize: 10
                    to: root.width
                    value: currentApplet && currentApplet.applet.configuration.length ? currentApplet.applet.configuration.length : 0
                    onValueModified: {
                        currentApplet.applet.configuration.length = value
                    }
                }
            }
        }
    }
}
