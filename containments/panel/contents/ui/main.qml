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
import QtQuick.Layouts 1.0
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.draganddrop 2.0 as DragDrop

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

DragDrop.DropArea {
    id: root
    width: 640
    height: 48

//BEGIN properties
    Layout.minimumWidth: currentLayout.Layout.minimumWidth
    Layout.maximumWidth: currentLayout.Layout.maximumWidth
    Layout.preferredWidth: currentLayout.Layout.preferredWidth

    Layout.minimumHeight: currentLayout.Layout.minimumHeight
    Layout.maximumHeight: currentLayout.Layout.maximumHeight
    Layout.preferredHeight: currentLayout.Layout.preferredHeight

    property Item toolBox

    property Item dragOverlay

    property bool isHorizontal: plasmoid.formFactor != PlasmaCore.Types.Vertical
//END properties

//BEGIN functions
function addApplet(applet, x, y) {
    var container = appletContainerComponent.createObject(root)
    print("Applet added in test panel: " + applet + applet.title + " at: " + x + ", " + y);

    applet.parent = container;
    container.applet = applet;
    applet.anchors.fill = container;
    applet.visible = true;
    container.visible = true;

    // Is there a DND placeholder? Replace it!
    if (dndSpacer.parent === currentLayout) {
        LayoutManager.insertBefore(dndSpacer, container);
        dndSpacer.parent = root;
        return;

    // If the provided position is valid, use it.
    } else if (x >= 0 && y >= 0) {
        var index = LayoutManager.insertAtCoordinates(container, x, y);
        print("Applet " + applet.id + " " + applet.title + " was added in position " + index);

    // Fall through to determining an appropriate insert position.
    } else {
        var before = null;

        // Insert icons to the left of whatever is at the center (usually a Task Manager),
        // if it exists.
        // FIXME TODO: This is a real-world fix to produce a sensible initial position for
        // launcher icons added by launcher menu applets. The basic approach has been used
        // since Plasma 1. However, "add launcher to X" is a generic-enough concept and
        // frequent-enough occurence that we'd like to abstract it further in the future
        // and get rid of the uglyness of parties external to the containment adding applets
        // of a specific type, and the containment caring about the applet type. In a better
        // system the containment would be informed of requested launchers, and determine by
        // itself what it wants to do with that information.
        if (applet.pluginName == "org.kde.plasma.icon") {
            var middle = currentLayout.childAt(root.width / 2, root.height / 2);

            if (middle) {
                before = middle;
            }

        // Otherwise if lastSpacer is here, enqueue after it.
        } else if (lastSpacer.parent === currentLayout) {
            before = lastSpacer;
        }

        if (before) {
            LayoutManager.insertBefore(before, container);

        // Fall through to adding at the end.
        } else {
            container.parent = currentLayout;
        }
    }

    if (applet.Layout.fillWidth) {
        lastSpacer.parent = root;
    }
}


function checkLastSpacer() {
    lastSpacer.parent = root

    var expands = false;

    if (isHorizontal) {
        for (var container in currentLayout.children) {
            var item = currentLayout.children[container];
            if (item.Layout && item.Layout.fillWidth) {
                expands = true;
            }
        }
    } else {
        for (var container in currentLayout.children) {
            var item = currentLayout.children[container];
            if (item.Layout && item.Layout.fillHeight) {
                expands = true;
            }
        }
    }
    if (!expands) {
        lastSpacer.parent = currentLayout
    }

}

//END functions

//BEGIN connections
    Component.onCompleted: {
        currentLayout.isLayoutHorizontal = isHorizontal
        LayoutManager.plasmoid = plasmoid;
        LayoutManager.root = root;
        LayoutManager.layout = currentLayout;
        LayoutManager.restore();
        containmentSizeSyncTimer.restart();
    }

    onDragEnter: {
        LayoutManager.insertAtCoordinates(dndSpacer, event.x, event.y)
    }

    onDragMove: {
        LayoutManager.insertAtCoordinates(dndSpacer, event.x, event.y)
    }

    onDragLeave: {
        dndSpacer.parent = root;
    }

    onDrop: {
        plasmoid.processMimeData(event.mimeData, event.x, event.y);
    }


    Containment.onAppletAdded: {
        addApplet(applet, x, y);
        LayoutManager.save();
    }

    Containment.onAppletRemoved: {
        var flexibleFound = false;
        for (var i = 0; i < currentLayout.children.length; ++i) {
            if (currentLayout.children[i].applet.Layout.fillWidth) {
                flexibleFound = true;
                break
            }
        }
        if (!flexibleFound) {
            lastSpacer.parent = currentLayout;
        }

        LayoutManager.save();
    }

    Plasmoid.onUserConfiguringChanged: {
        if (plasmoid.immutable) {
            if (dragOverlay) {
                dragOverlay.destroy();
            }
            return;
        }

        if (plasmoid.userConfiguring) {
            if (!dragOverlay) {
                var component = Qt.createComponent("ConfigOverlay.qml");
                dragOverlay = component.createObject(root);
                component.destroy();
            } else {
                dragOverlay.visible = true;
            }
        } else {
            dragOverlay.visible = false;
            dragOverlay.destroy();
        }
    }

    Plasmoid.onFormFactorChanged: containmentSizeSyncTimer.restart();
    Plasmoid.onImmutableChanged: containmentSizeSyncTimer.restart();
    onToolBoxChanged: containmentSizeSyncTimer.restart();
//END connections

//BEGIN components
    Component {
        id: appletContainerComponent
        Item {
            id: container
            visible: false

            Layout.fillWidth: applet && applet.Layout.fillWidth
            Layout.onFillWidthChanged: {
                if (plasmoid.formFactor != PlasmaCore.Types.Vertical) {
                    checkLastSpacer();
                }
            }
            Layout.fillHeight: applet && applet.Layout.fillHeight
            Layout.onFillHeightChanged: {
                if (plasmoid.formFactor == PlasmaCore.Types.Vertical) {
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

            PlasmaComponents.BusyIndicator {
                z: 1000
                visible: applet && applet.busy
                running: visible
                anchors.centerIn: parent
            }
            onXChanged: {
                translation.x = oldX - x
                translation.y = oldY - y
                translAnim.running = true
                oldX = x
                oldY = y
            }
            onYChanged: {
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
        width: (plasmoid.formFactor == PlasmaCore.Types.Vertical) ? currentLayout.width : theme.mSize(theme.defaultFont).width * 10
        height: (plasmoid.formFactor == PlasmaCore.Types.Vertical) ?  theme.mSize(theme.defaultFont).width * 10 : currentLayout.height
    }

    GridLayout {
        id: currentLayout
        property bool isLayoutHorizontal

        Layout.preferredWidth: {
            var width = 0;
            for (var i = 0; i < currentLayout.children.length; ++i) {
                if (currentLayout.children[i].Layout) {
                    width += Math.max(currentLayout.children[i].Layout.minimumWidth, currentLayout.children[i].Layout.preferredWidth);
                }
            }
            return width;
        }
        Layout.preferredHeight: {
            var height = 0;
            for (var i = 0; i < currentLayout.children.length; ++i) {
                if (currentLayout.children[i].Layout) {
                    height += Math.max(currentLayout.children[i].Layout.minimumHeight, currentLayout.children[i].Layout.preferredHeight);
                }
            }
            return height;
        }
        rows: 1
        columns: 1
        //when horizontal layout top-to-bottom, this way it will obey our limit of one row and actually lay out left to right
        flow: isHorizontal ? GridLayout.TopToBottom : GridLayout.LeftToRight
    }

    onWidthChanged: {
        containmentSizeSyncTimer.restart()
    }
    onHeightChanged: {
        containmentSizeSyncTimer.restart()
    }

    Timer {
        id: containmentSizeSyncTimer
        interval: 150
        onTriggered: {
            currentLayout.x = 0
            currentLayout.y = 0
            currentLayout.width = root.width - (isHorizontal && toolBox && toolBox.visible ? toolBox.width : 0)
            currentLayout.height = root.height - (!isHorizontal && toolBox && toolBox.visible ? toolBox.height : 0)
            currentLayout.isLayoutHorizontal = isHorizontal
        }
    }
//END UI elements
}
