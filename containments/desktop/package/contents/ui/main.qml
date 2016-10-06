/***************************************************************************
 *   Copyright (C) 2011-2013 Sebastian Kügler <sebas@kde.org>              *
 *   Copyright (C) 2011 Marco Martin <mart@kde.org>                        *
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.4
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.draganddrop 2.0 as DragDrop
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

import org.kde.private.desktopcontainment.desktop 0.1 as Desktop

import "LayoutManager.js" as LayoutManager
import "FolderTools.js" as FolderTools

DragDrop.DropArea {
    id: root
    objectName: isFolder ? "folder" : "desktop"

    width: isPopup ? undefined : preferredWidth(false) // for the initial size when placed on the desktop
    Layout.minimumWidth: preferredWidth(true)
    Layout.preferredWidth: isPopup ? preferredWidth(false) : 0 // for the popup size to change at runtime when view mode changes
    Plasmoid.switchWidth: preferredWidth(true)

    height: isPopup ? undefined : preferredHeight(false)
    Layout.minimumHeight: preferredHeight(true)
    Layout.preferredHeight: isPopup ? preferredHeight(false) : 0
    Plasmoid.switchHeight: preferredHeight(true)

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    property bool isFolder: (plasmoid.pluginName == "org.kde.plasma.folder")
    property bool isContainment: ("containmentType" in plasmoid)
    property bool isPopup: (plasmoid.location != PlasmaCore.Types.Floating)
    property bool useListViewMode: isPopup && plasmoid.configuration.viewMode === 0

    property Component appletAppearanceComponent
    property Item toolBox
    property var layoutManager: LayoutManager

    property int handleDelay: 800
    property real haloOpacity: 0.5

    property int iconSize: units.iconSizes.small
    property int iconWidth: iconSize
    property int iconHeight: iconWidth

    preventStealing: true

    // Plasmoid.title is set by a Binding {} in FolderViewLayer
    Plasmoid.toolTipSubText: ""
    Plasmoid.icon: (!plasmoid.configuration.useCustomIcon && folderViewLayer.ready) ? folderViewLayer.view.model.iconName : plasmoid.configuration.icon
    Plasmoid.compactRepresentation: (isFolder && !isContainment) ? compactRepresentation : null
    Plasmoid.associatedApplicationUrls: folderViewLayer.ready ? folderViewLayer.model.resolvedUrl : null

    onIconHeightChanged: updateGridSize()

    anchors {
        leftMargin: (isContainment && plasmoid.availableScreenRect) ? plasmoid.availableScreenRect.x : 0
        topMargin: (isContainment && plasmoid.availableScreenRect) ? plasmoid.availableScreenRect.y : 0

        // Don't apply the right margin if the folderView is in column mode and not overflowing.
        // In this way, the last column remains droppable even if a small part of the icon is behind a panel.
        rightMargin: folderViewLayer.ready && (folderViewLayer.view.overflowing  || folderViewLayer.view.flow == GridView.FlowLeftToRight)
            && (isContainment && plasmoid.availableScreenRect) && parent
            ? parent.width - (plasmoid.availableScreenRect.x + plasmoid.availableScreenRect.width) : 0

        // Same mechanism as the right margin but applied here to the bottom when the folderView is in row mode.
        bottomMargin: folderViewLayer.ready && (folderViewLayer.view.overflowing || folderViewLayer.view.flow == GridView.FlowTopToBottom)
            && (isContainment && plasmoid.availableScreenRect) && parent
            ? parent.height - (plasmoid.availableScreenRect.y + plasmoid.availableScreenRect.height) : 0
    }

    Behavior on anchors.topMargin {
        NumberAnimation { duration: units.longDuration; easing.type: Easing.InOutQuad }
    }
    Behavior on anchors.leftMargin {
        NumberAnimation { duration: units.longDuration; easing.type: Easing.InOutQuad }
    }
    Behavior on anchors.rightMargin {
        NumberAnimation { duration: units.longDuration; easing.type: Easing.InOutQuad }
    }
    Behavior on anchors.bottomMargin {
        NumberAnimation { duration: units.longDuration; easing.type: Easing.InOutQuad }
    }

    function updateGridSize()
    {
        LayoutManager.cellSize.width = root.iconWidth + toolBoxSvg.elementSize("left").width + toolBoxSvg.elementSize("right").width
        LayoutManager.cellSize.height = root.iconHeight + toolBoxSvg.elementSize("top").height + toolBoxSvg.elementSize("bottom").height;
        LayoutManager.defaultAppletSize.width = LayoutManager.cellSize.width * 6;
        LayoutManager.defaultAppletSize.height = LayoutManager.cellSize.height * 6;
        layoutTimer.restart();
    }

    function addLauncher(desktopUrl) {
        if (!isFolder) {
            return;
        }

        folderViewLayer.view.linkHere(desktopUrl);
    }

    function addApplet(applet, x, y) {
        if (!appletAppearanceComponent) {
            appletAppearanceComponent = Qt.createComponent("AppletAppearance.qml");
        }

        if (appletAppearanceComponent.status !== Component.Ready) {
            console.warn("Error loading AppletAppearance.qml:", appletAppearanceComponent.errorString());
            return;
        }

        var category = "Applet-" + applet.id;

        var container = appletAppearanceComponent.createObject(resultsFlow, {
            category: category
        });

        applet.parent = container
        applet.visible = true;

        var config = LayoutManager.itemsConfig[category];

        // We have it in the config.
        if (config !== undefined && config.width !== undefined &&
            config.height !== undefined &&
            config.width > 0 && config.height > 0) {
            container.width = config.width;
            container.height = config.height;
        // We have a default.
        } else if (applet.width > 0 && applet.height > 0) {
            container.width = applet.width;
            container.height = applet.height;
            // The container needs to be bigger than applet of margins factor.
            if (applet.backgroundHints != PlasmaCore.Types.NoBackground) {
                container.width += container.margins.left + container.margins.right;
                container.height += container.margins.top + container.margins.bottom;
            }
        // Give up, assign the global default.
        } else {
            container.width = LayoutManager.defaultAppletSize.width;
            container.height = LayoutManager.defaultAppletSize.height;
        }

        container.applet = applet;

        // Coordinated passed by param?
        if ( x >= 0 && y >= 0) {
            if (x + container.width > root.width) {
                x = root.width - container.width - 10;
            }
            if (y + container.height > root.height) {
                x = root.height - container.height;
            }

            // On applet undo or via scripting, the applet position will be saved
            // in applet's scene coordinates so remap it to resultsflow's coordinates.
            var pos = root.parent.mapToItem(resultsFlow, x, y);

            container.x = pos.x;
            container.y = pos.y;

            // To be sure it's restored at the same position, take margins into account
            // if there is a background.
            if (applet.backgroundHints != PlasmaCore.Types.NoBackground) {
                container.x -= container.margins.left;
                container.y -= container.margins.top;
            }

        // Coordinates stored?
        } else if (config !== undefined && config.x !== undefined && config.y !== undefined &&
            config.x >= 0 && config.y >= 0) {
            container.x = config.x;
            container.y = config.y;
        }

        // Rotation stored and significant?
        if (config !== undefined && config.rotation !== undefined &&
            (config.rotation > 5 || config.rotation < -5)) {
            container.rotation = config.rotation;
        } else {
            LayoutManager.restoreRotation(container);
        }

        LayoutManager.itemGroups[container.category] = container;

        if (container.x >= 0 && container.y >= 0) {
            LayoutManager.positionItem(container);
        }
    }

    function preferredWidth(minimum) {
        if (isContainment || !folderViewLayer.ready) {
            return -1;
        } else if (useListViewMode) {
            return (minimum ? folderViewLayer.view.cellHeight * 4 : units.gridUnit * 16);
        }

        return (folderViewLayer.view.cellWidth * (minimum ? 1 : 3)) + (units.largeSpacing * 2);
    }

    function preferredHeight(minimum) {
        if (isContainment || !folderViewLayer.ready) {
            return -1;
        } else if (useListViewMode) {
            var height = (folderViewLayer.view.cellHeight * (minimum ? 1 : 15)) + units.smallSpacing;
        } else {
            var height = (folderViewLayer.view.cellHeight * (minimum ? 1 : 2)) + units.largeSpacing
        }

        if (plasmoid.configuration.labelMode != 0) {
            height += folderViewLayer.item.labelHeight;
        }

        return height;
    }

    function isDrag(fromX, fromY, toX, toY) {
        var length = Math.abs(fromX - toX) + Math.abs(fromY - toY);
        return length >= Qt.styleHints.startDragDistance;
    }

    onDragEnter: {
        if (isContainment && plasmoid.immutable && !(isFolder && FolderTools.isFileDrag(event))) {
            event.ignore();
        }
    }

    onDragMove: {
        // TODO: We should reject drag moves onto file items that don't accept drops
        // (cf. QAbstractItemModel::flags() here, but DeclarativeDropArea currently
        // is currently incapable of rejecting drag events.

        // Trigger autoscroll.
        if (isFolder && FolderTools.isFileDrag(event)) {
            folderViewLayer.view.scrollLeft = (event.x < (units.largeSpacing * 3));
            folderViewLayer.view.scrollRight = (event.x > width - (units.largeSpacing * 3));
            folderViewLayer.view.scrollUp = (event.y < (units.largeSpacing * 3));
            folderViewLayer.view.scrollDown = (event.y > height - (units.largeSpacing * 3));
        } else if (isContainment) {
            placeHolder.width = LayoutManager.defaultAppletSize.width;
            placeHolder.height = LayoutManager.defaultAppletSize.height;
            placeHolder.minimumWidth = placeHolder.minimumHeight = 0;
            placeHolder.x = event.x - placeHolder.width / 2;
            placeHolder.y = event.y - placeHolder.width / 2;
            LayoutManager.positionItem(placeHolder);
            LayoutManager.setSpaceAvailable(placeHolder.x, placeHolder.y, placeHolder.width, placeHolder.height, true);
            placeHolderPaint.opacity = root.haloOpacity;
        }
    }

    onDragLeave: {
        // Cancel autoscroll.
        if (isFolder) {
            folderViewLayer.view.scrollLeft = false;
            folderViewLayer.view.scrollRight = false;
            folderViewLayer.view.scrollUp = false;
            folderViewLayer.view.scrollDown = false;
        }

        if (isContainment) {
            placeHolderPaint.opacity = 0;
        }
    }

    onDrop: {
        if (isFolder && FolderTools.isFileDrag(event)) {
            // Cancel autoscroll.
            folderViewLayer.view.scrollLeft = false;
            folderViewLayer.view.scrollRight = false;
            folderViewLayer.view.scrollUp = false;
            folderViewLayer.view.scrollDown = false;

            folderViewLayer.view.drop(root, event, mapToItem(folderViewLayer.view, event.x, event.y));
        } else if (isContainment) {
            placeHolderPaint.opacity = 0;
            var pos = root.parent.mapFromItem(resultsFlow, event.x - placeHolder.width / 2, event.y - placeHolder.height / 2);
            plasmoid.processMimeData(event.mimeData, pos.x, pos.y);
            event.accept(event.proposedAction);
        }
    }

    Component {
        id: compactRepresentation
        CompactRepresentation { folderView: folderViewLayer.view }
    }

    Connections {
        target: plasmoid

        ignoreUnknownSignals: true

        onAppletAdded: {
            addApplet(applet, x, y);
            // Clean any eventual invalid chunks in the config.
            LayoutManager.save();
        }

        onAppletRemoved: {
            // Clean any eventual invalid chunks in the config.
            LayoutManager.removeApplet(applet);
            LayoutManager.save();
        }

        onImmutableChanged: {
            if (!plasmoid.immutable) {
                pressToMoveHelp.show();
            }
        }
        
        onAvailableScreenRegionChanged: layoutTimer.restart();
    }

    Connections {
        target: plasmoid.configuration

        onPressToMoveChanged: {
            if (plasmoid.configuration.pressToMove && plasmoid.configuration.pressToMoveHelp && !plasmoid.immutable) {
                pressToMoveHelp.show();
            }
        }
    }

    Binding {
        target: toolBox
        property: "visible"
        value: plasmoid.configuration.showToolbox
    }

    Desktop.InfoNotification {
        id: pressToMoveHelp

        enabled: plasmoid.configuration.pressToMove && plasmoid.configuration.pressToMoveHelp

        iconName: "plasma"
        titleText: i18n("Widgets unlocked")
        text: i18n("You can press and hold widgets to move them and reveal their handles.")
        acknowledgeActionText: i18n("Got it")

        onAcknowledged: {
            plasmoid.configuration.pressToMoveHelp = false;
        }
    }

    PlasmaCore.FrameSvgItem {
        id : highlightItemSvg

        visible: false

        imagePath: isPopup ? "widgets/viewitem" : ""
        prefix: "hover"
    }

    PlasmaCore.FrameSvgItem {
        id : listItemSvg

        visible: false

        imagePath: isPopup ? "widgets/viewitem" : ""
        prefix: "normal"
    }

    PlasmaCore.Svg {
        id: toolBoxSvg
        imagePath: "widgets/toolbox"
        property int rightBorder: elementSize("right").width
        property int topBorder: elementSize("top").height
        property int bottomBorder: elementSize("bottom").height
        property int leftBorder: elementSize("left").width
    }

    PlasmaCore.Svg {
        id: configIconsSvg
        imagePath: "widgets/configuration-icons"
    }

    KQuickControlsAddons.EventGenerator {
        id: eventGenerator
    }

    MouseArea { // unfocus any plasmoid when clicking empty desktop area
        anchors.fill: parent
        onPressed: {
            root.forceActiveFocus()
            mouse.accepted = false // Bug 351277
        }
    }

    Loader {
        id: folderViewLayer

        anchors.fill: parent

        property bool ready: status == Loader.Ready
        property Item view: item ? item.view : null
        property QtObject model: item ? item.model : null

        focus: true

        active: isFolder
        asynchronous: false

        source: "FolderViewLayer.qml"

        onFocusChanged: {
            if (!focus && model) {
                model.clearSelection();
            }
        }

        Connections {
            target: folderViewLayer.view

            onPressed: {
                folderViewLayer.focus = true;
            }
        }
    }

    Item {
        id: resultsFlow
        anchors.fill: parent

        anchors {
            top: parent.top
            topMargin: 5
            horizontalCenter: parent.horizontalCenter
        }

        visible: isContainment
        enabled: isContainment

        // This is just for event compression when a lot of boxes are created one after the other.
        Timer {
            id: layoutTimer
            repeat: false
            running: false
            interval: 100
            onTriggered: {
                LayoutManager.resetPositions()
                for (var i=0; i<resultsFlow.children.length; ++i) {
                    var child = resultsFlow.children[i]
                    if (!child.applet)
                        continue
                    if (child.enabled) {
                        if (LayoutManager.itemsConfig[child.category]) {
                            var rect = LayoutManager.itemsConfig[child.category]
                            child.x = rect.x
                            child.y = rect.y
                            child.width = rect.width
                            child.height = rect.height
                            child.rotation = rect.rotation
                        } else {
                            child.x = 0
                            child.y = 0
                            child.width = Math.min(470, 32+child.categoryCount*140)
                        }
                        child.visible = true
                        LayoutManager.positionItem(child)
                    } else {
                        child.visible = false
                    }
                }
                LayoutManager.save()
            }
        }
    }

    Item {
        id: placerHolderWrapper

        anchors.fill: resultsFlow
        z: 0

        visible: isContainment
        enabled: isContainment

        Item {
            id: placeHolder

            x: -10000 // Move offscreen initially to avoid flickering.
            width: 100
            height: 100

            property bool animationsEnabled
            property int minimumWidth
            property int minimumHeight
            property Item syncItem

            function syncWithItem(item) {
                syncItem = item;
                minimumWidth = item.minimumWidth;
                minimumHeight = item.minimumHeight;
                repositionTimer.running = true;
                if (placeHolderPaint.opacity < 1) {
                    placeHolder.delayedSyncWithItem();
                }
            }

            function delayedSyncWithItem() {
                placeHolder.x = placeHolder.syncItem.x;
                placeHolder.y = placeHolder.syncItem.y;
                placeHolder.width = placeHolder.syncItem.width + (plasmoid.immutable || !syncItem.showAppletHandle ? 0 : syncItem.handleWidth)
                placeHolder.height = placeHolder.syncItem.height;
                // Only positionItem here, we don't want to save.
                LayoutManager.positionItem(placeHolder);
                LayoutManager.setSpaceAvailable(placeHolder.x, placeHolder.y, placeHolder.width, placeHolder.height, true);
            }

            Timer {
                id: repositionTimer
                interval: 100
                repeat: false
                running: false
                onTriggered: placeHolder.delayedSyncWithItem()
            }
        }

        PlasmaComponents.Highlight {
            id: placeHolderPaint

            x: placeHolder.x
            y: placeHolder.y
            width: placeHolder.width
            height: placeHolder.height
            z: 0

            opacity: 0
            visible: opacity > 0

            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    Component.onCompleted: {
        if (!isContainment) {
            return;
        }

        plasmoid.action("configure").text = i18n("Configure Desktop");

        // WORKAROUND: that's the only place where we can inject a sensible size.
        // if root has width defined, it will override the value we set before
        // the component completes
        root.width = plasmoid.width;

        LayoutManager.resultsFlow = resultsFlow;
        LayoutManager.plasmoid = plasmoid;
        updateGridSize();

        LayoutManager.restore();

        for (var i = 0; i < plasmoid.applets.length; ++i) {
            var applet = plasmoid.applets[i];
            addApplet(applet, -1, -1);
        }

        // Clean any eventual invalid chunks in the config.
        LayoutManager.save();
    }
}
