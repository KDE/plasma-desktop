/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.draganddrop 2.0 as DragDrop

import "LayoutManager.js" as LayoutManager

DragDrop.DropArea {
    id: root
    width: 640
    height: 48

//BEGIN properties
    Layout.minimumWidth: fixedWidth > 0 ? fixedWidth : (currentLayout.Layout.minimumWidth + (isHorizontal && toolBox ? toolBox.width : 0))
    Layout.maximumWidth: fixedWidth > 0 ? fixedWidth : (currentLayout.Layout.maximumWidth + (isHorizontal && toolBox ? toolBox.width : 0))
    Layout.preferredWidth: fixedWidth > 0 ? fixedWidth : (currentLayout.Layout.preferredWidth + (isHorizontal && toolBox ? toolBox.width : 0))

    Layout.minimumHeight: fixedHeight > 0 ? fixedHeight : (currentLayout.Layout.minimumHeight + (!isHorizontal && toolBox ? toolBox.height : 0))
    Layout.maximumHeight: fixedHeight > 0 ? fixedHeight : (currentLayout.Layout.maximumHeight + (!isHorizontal && toolBox ? toolBox.height : 0))
    Layout.preferredHeight: fixedHeight > 0 ? fixedHeight : (currentLayout.Layout.preferredHeight + (!isHorizontal && toolBox? toolBox.height : 0))

    property Item toolBox
    property var layoutManager: LayoutManager

    property Item dragOverlay

    property bool isHorizontal: plasmoid.formFactor !== PlasmaCore.Types.Vertical
    property int fixedWidth: 0
    property int fixedHeight: 0

//END properties

//BEGIN functions
function addApplet(applet, x, y) {
    // don't show applet if it chooses to be hidden but still make it
    // accessible in the panelcontroller
    // Due to the nature of how "visible" propagates in QML, we need to
    // explicitly set it on the container (so the Layout ignores it)
    // as well as the applet (so it reliably knows about), otherwise it can
    // happen that an applet erroneously thinks it's visible, or suddenly
    // starts thinking that way on teardown (virtual desktop pager)
    // leading to crashes
    var visibleBinding = Qt.binding(function() {
        return applet.status !== PlasmaCore.Types.HiddenStatus || (!plasmoid.immutable && plasmoid.userConfiguring);
    })

    var container = appletContainerComponent.createObject(root, {
        applet: applet,
        visible: visibleBinding
    });

    applet.parent = container;
    applet.anchors.fill = container;

    applet.visible = visibleBinding;

    // Is there a DND placeholder? Replace it!
    if (dndSpacer.parent === currentLayout) {
        LayoutManager.insertBefore(dndSpacer, container);
        dndSpacer.parent = root;
        return;

    // If the provided position is valid, use it.
    } else if (x >= 0 && y >= 0) {
        var index = LayoutManager.insertAtCoordinates(container, x , y);

    // Fall through to determining an appropriate insert position.
    } else {
        var before = lastSpacer;
        container.animationsEnabled = false;

        // Insert icons to the left of whatever is at the center (usually a Task Manager),
        // if it exists.
        // FIXME TODO: This is a real-world fix to produce a sensible initial position for
        // launcher icons added by launcher menu applets. The basic approach has been used
        // since Plasma 1. However, "add launcher to X" is a generic-enough concept and
        // frequent-enough occurrence that we'd like to abstract it further in the future
        // and get rid of the ugliness of parties external to the containment adding applets
        // of a specific type, and the containment caring about the applet type. In a better
        // system the containment would be informed of requested launchers, and determine by
        // itself what it wants to do with that information.
        if (!startupTimer.running && applet.pluginName === "org.kde.plasma.icon") {
            var middle = currentLayout.childAt(root.width / 2, root.height / 2);

            if (middle) {
                before = middle;
            }

        // lastSpacer is here, enqueue before it.
        }


        LayoutManager.insertBefore(before, container);

        //event compress the enable of animations
        startupTimer.restart();
    }
}


function checkLastSpacer() {
    var flexibleFound = false;
    for (var i = 0; i < currentLayout.children.length; ++i) {
        var applet = currentLayout.children[i].applet;
        if (!applet) {
            continue;
        }
        if (!applet.visible || !applet.Layout) {
            continue;
        }
        if ((root.isHorizontal && applet.Layout.fillWidth) ||
            (!root.isHorizontal && applet.Layout.fillHeight)) {
                flexibleFound = true;
            break
        }
    }
    lastSpacer.visible= !flexibleFound;
}
//END functions

//BEGIN connections
    Component.onCompleted: {
        currentLayout.isLayoutHorizontal = isHorizontal
        LayoutManager.plasmoid = plasmoid;
        LayoutManager.root = root;
        LayoutManager.layout = currentLayout;
        LayoutManager.lastSpacer = lastSpacer;
        LayoutManager.restore();
        containmentSizeSyncTimer.restart();

        plasmoid.action("configure").visible = Qt.binding(function() {
            return !plasmoid.immutable;
        });
        plasmoid.action("configure").enabled = Qt.binding(function() {
            return !plasmoid.immutable;
        });
    }

    onDragEnter: {
        if (plasmoid.immutable) {
            event.ignore();
            return;
        }
        //during drag operations we disable panel auto resize
        if (root.isHorizontal) {
            root.fixedWidth = root.width
        } else {
            root.fixedHeight = root.height
        }
        LayoutManager.insertAtCoordinates(dndSpacer, event.x, event.y)
    }

    onDragMove: {
        LayoutManager.insertAtCoordinates(dndSpacer, event.x, event.y)
    }

    onDragLeave: {
        dndSpacer.parent = root;
        root.fixedWidth = 0;
        root.fixedHeight = 0;
    }

    onDrop: {
        plasmoid.processMimeData(event.mimeData, event.x, event.y);
        event.accept(event.proposedAction);
        root.fixedWidth = 0;
        root.fixedHeight = 0;
        containmentSizeSyncTimer.restart();
    }


    Containment.onAppletAdded: {
        addApplet(applet, x, y);
        checkLastSpacer();
        LayoutManager.save();
    }

    Containment.onAppletRemoved: {
        LayoutManager.removeApplet(applet);
        checkLastSpacer();
        LayoutManager.save();
    }

    Plasmoid.onUserConfiguringChanged: {
        containmentSizeSyncTimer.restart();

        if (plasmoid.immutable) {
            if (dragOverlay) {
                dragOverlay.destroy();
            }
            return;
        }

        if (plasmoid.userConfiguring) {
            for (var i = 0; i < plasmoid.applets.length; ++i) {
                plasmoid.applets[i].expanded = false;
            }

            if (!dragOverlay) {
                var component = Qt.createComponent("ConfigOverlay.qml");
                if (component.status === Component.Ready) {
                    dragOverlay = component.createObject(root);
                } else {
                    console.log("Could not create ConfigOverlay:", component.errorString());
                }
                component.destroy();
            } else {
                dragOverlay.visible = true;
            }
        } else {
            dragOverlay.destroy();
        }
    }

    Plasmoid.onFormFactorChanged: containmentSizeSyncTimer.restart();
    Containment.onEditModeChanged: containmentSizeSyncTimer.restart();

    onToolBoxChanged: {
        containmentSizeSyncTimer.restart();
        if (startupTimer.running) {
            startupTimer.restart();
        }
    }
//END connections

//BEGIN components
    Component {
        id: appletContainerComponent
        // This loader conditionally manages the BusyIndicator, it's not
        // loading the applet. The applet becomes a regular child item.
        Loader {
            id: container
            visible: false
            property bool animationsEnabled: true

            //when the applet moves caused by its resize, don't animate.
            //this is completely heuristic, but looks way less "jumpy"
            property bool movingForResize: false

            Layout.fillWidth: applet && applet.Layout.fillWidth
            Layout.onFillWidthChanged: {
                if (plasmoid.formFactor !== PlasmaCore.Types.Vertical) {
                    checkLastSpacer();
                }
            }
            Layout.fillHeight: applet && applet.Layout.fillHeight
            Layout.onFillHeightChanged: {
                if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    checkLastSpacer();
                }
            }

            Layout.minimumWidth: (currentLayout.isLayoutHorizontal ? (applet && applet.Layout.minimumWidth > 0 ? applet.Layout.minimumWidth : root.height) : root.width)
            Layout.minimumHeight: (!currentLayout.isLayoutHorizontal ? (applet && applet.Layout.minimumHeight > 0 ? applet.Layout.minimumHeight : root.width) : root.height)

            Layout.preferredWidth: (currentLayout.isLayoutHorizontal ? (applet && applet.Layout.preferredWidth > 0 ? applet.Layout.preferredWidth : root.height) : root.width)
            Layout.preferredHeight: (!currentLayout.isLayoutHorizontal ? (applet && applet.Layout.preferredHeight > 0 ? applet.Layout.preferredHeight : root.width) : root.height)

            Layout.maximumWidth: (currentLayout.isLayoutHorizontal ? (applet && applet.Layout.maximumWidth > 0 ? applet.Layout.maximumWidth : (Layout.fillWidth ? root.width : root.height)) : root.height)
            Layout.maximumHeight: (!currentLayout.isLayoutHorizontal ? (applet && applet.Layout.maximumHeight > 0 ? applet.Layout.maximumHeight : (Layout.fillHeight ? root.height : root.width)) : root.width)

            property int oldX: x
            property int oldY: y

            property Item applet

            onAppletChanged: {
                if (!applet) {
                    destroy();
                }
            }

            active: applet && applet.busy
            sourceComponent: PlasmaComponents.BusyIndicator {}

            Layout.onMinimumWidthChanged: movingForResize = true;
            Layout.onMinimumHeightChanged: movingForResize = true;
            Layout.onMaximumWidthChanged: movingForResize = true;
            Layout.onMaximumHeightChanged: movingForResize = true;

            onXChanged: {
                if (movingForResize) {
                    movingForResize = false;
                    return;
                }
                if (!animationsEnabled) {
                    startupTimer.restart();
                    return;
                }
                translation.x = oldX - x
                translation.y = oldY - y
                translAnim.running = true
                oldX = x
                oldY = y
            }
            onYChanged: {
                if (movingForResize) {
                    movingForResize = false;
                    return;
                }
                if (!animationsEnabled) {
                    startupTimer.restart();
                    return;
                }
                translation.x = oldX - x
                translation.y = oldY - y
                translAnim.running = true
                oldX = x
                oldY = y
            }
            transform: Translate {
                id: translation
            }
            NumberAnimation {
                id: translAnim
                duration: units.longDuration
                easing.type: Easing.InOutQuad
                target: translation
                properties: "x,y"
                to: 0
            }
        }
    }
//END components

//BEGIN UI elements
    Item {
        id: lastSpacer
        parent: currentLayout

        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    Item {
        id: dndSpacer
        Layout.preferredWidth: width
        Layout.preferredHeight: height
        width: (plasmoid.formFactor === PlasmaCore.Types.Vertical) ? currentLayout.width : theme.mSize(theme.defaultFont).width * 10
        height: (plasmoid.formFactor === PlasmaCore.Types.Vertical) ?  theme.mSize(theme.defaultFont).width * 10 : currentLayout.height
    }

    // while the user is moving the applet when configuring the panel, the applet is reparented
    // here so it can be moved freely; previously it was reparented to "root" but this one does not
    // take into account the toolbox (which is left-of) the layout in right-to-left languages
    Item {
        id: moveAppletLayer
        anchors.fill: currentLayout
    }

    GridLayout {
        id: currentLayout
        property bool isLayoutHorizontal
        rowSpacing: units.smallSpacing
        columnSpacing: units.smallSpacing

        Layout.preferredWidth: {
            var width = 0;
            for (var i = 0, length = currentLayout.children.length; i < length; ++i) {
                var item = currentLayout.children[i];
                if (item.Layout) {
                    width += Math.max(item.Layout.minimumWidth, item.Layout.preferredWidth);
                }
            }
            return width;
        }
        Layout.preferredHeight: {
            var height = 0;
            for (var i = 0, length = currentLayout.children.length; i < length; ++i) {
                var item = currentLayout.children[i];
                if (item.Layout) {
                    height += Math.max(item.Layout.minimumHeight, item.Layout.preferredHeight);
                }
            }
            return height;
        }
        rows: 1
        columns: 1
        //when horizontal layout top-to-bottom, this way it will obey our limit of one row and actually lay out left to right
        flow: isHorizontal ? GridLayout.TopToBottom : GridLayout.LeftToRight
        layoutDirection: Qt.application.layoutDirection
    }

    onWidthChanged: {
        containmentSizeSyncTimer.restart()
        if (startupTimer.running) {
            startupTimer.restart();
        }
    }
    onHeightChanged: {
        containmentSizeSyncTimer.restart()
        if (startupTimer.running) {
            startupTimer.restart();
        }
    }

    Timer {
        id: containmentSizeSyncTimer
        interval: 150
        onTriggered: {
            dndSpacer.parent = root;
            currentLayout.x = (isHorizontal && toolBox && Qt.application.layoutDirection === Qt.RightToLeft && plasmoid.editMode) ? toolBox.width : 0;
            currentLayout.y = 0
            currentLayout.width = root.width - (isHorizontal && toolBox && plasmoid.editMode ? toolBox.width : 0)
            currentLayout.height = root.height - (!isHorizontal && toolBox && plasmoid.editMode ? toolBox.height : 0)
            currentLayout.isLayoutHorizontal = isHorizontal
        }
    }

    //FIXME: I don't see other ways at the moment a way to see when the UI is REALLY ready
    Timer {
        id: startupTimer
        interval: 4000
        onTriggered: {
            for (var i = 0; i < currentLayout.children.length; ++i) {
                var item = currentLayout.children[i];
                if (item.hasOwnProperty("animationsEnabled")) {
                    item.animationsEnabled = true;
                }
            }
        }
    }
//END UI elements
}
