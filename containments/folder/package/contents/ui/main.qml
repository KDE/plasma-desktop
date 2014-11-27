/***************************************************************************
 *   Copyright (C) 2011 Marco Martin <mart@kde.org>                        *
 *   Copyright (C) 2011-2013 Sebastian Kügler <sebas@kde.org>              *
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.draganddrop 2.0 as DragDrop

import org.kde.plasma.private.folder 0.1 as Folder

import "plasmapackage:/code/LayoutManager.js" as LayoutManager

DragDrop.DropArea {
    id: root
    objectName: "folder"

    width: isContainment ? undefined : (itemView.cellWidth * 3) + (units.largeSpacing * 3)
    height: isContainment ? undefined : (itemView.cellHeight * 2) + (units.largeSpacing * 2)

    property bool isContainment: ("containmentType" in plasmoid)
    property Item label: null
    property Item toolBox
    property variant sharedActions: ["newMenu", "paste", "undo", "refresh", "emptyTrash"]
    property Component itemViewDialogComponent: Qt.createComponent("ItemViewDialog.qml", Qt.Asynchronous, root)

    property bool debug: false
    property int handleDelay: 800
    property real haloOpacity: 0.5
    property bool pressAndHoldHandle: false

    property int iconSize: 16
    property int iconWidth: iconSize
    property int iconHeight: iconWidth

    preventStealing: true
    enabled: true

    Plasmoid.associatedApplicationUrls: itemView.model.resolvedUrl

    onIconHeightChanged: updateGridSize()

    anchors {
        leftMargin: plasmoid.availableScreenRect ? plasmoid.availableScreenRect.x : 0
        topMargin: plasmoid.availableScreenRect ? plasmoid.availableScreenRect.y : 0

        rightMargin: plasmoid.availableScreenRect && parent ? parent.width - (plasmoid.availableScreenRect.x + plasmoid.availableScreenRect.width) : 0
        bottomMargin: itemView.overflowing && plasmoid.availableScreenRect && parent ? parent.height - (plasmoid.availableScreenRect.y + plasmoid.availableScreenRect.height) : 0
    }

    function updateContextualActions() {
        itemView.model.updateActions();

        var actionName = "";
        var appletAction = null;
        var modelAction = null;

        for (var i = 0; i < sharedActions.length; i++) {
            actionName = sharedActions[i];
            appletAction = plasmoid.action(actionName);
            modelAction = itemView.model.action(actionName);

            appletAction.text = modelAction.text;
            appletAction.enabled = modelAction.enabled;
            appletAction.visible = modelAction.visible;
        }
    }

    function updateGridSize()
    {
//         print("Updategridsize");
        LayoutManager.cellSize.width = root.iconWidth + toolBoxSvg.elementSize("left").width + toolBoxSvg.elementSize("right").width
        LayoutManager.cellSize.height = root.iconHeight + toolBoxSvg.elementSize("top").height + toolBoxSvg.elementSize("bottom").height;
        LayoutManager.defaultAppletSize.width = LayoutManager.cellSize.width * 6;
        LayoutManager.defaultAppletSize.height = LayoutManager.cellSize.height * 6;
        layoutTimer.restart()
    }

    function addApplet(applet, x, y) {
        var component = Qt.createComponent("AppletAppearance.qml");
        var e = component.errorString();
        if (e != "") {
            print("Error loading AppletAppearance.qml: " + component.errorString());
        }

        var container = component.createObject(resultsFlow)

        applet.parent = container
        applet.visible = true;

        container.category = "Applet-"+applet.id;
        var config = LayoutManager.itemsConfig[container.category];

        //we have it in the config
        if (config !== undefined && config.width !== undefined &&
            config.height !== undefined &&
            config.width > 0 && config.height > 0) {
            container.width = config.width;
            container.height = config.height;
        //we have a default
        } else if (applet.width > 0 && applet.height > 0) {
            container.width = applet.width;
            container.height = applet.height;
        //give up, assign the global default
        } else {
            container.width = LayoutManager.defaultAppletSize.width;
            container.height = LayoutManager.defaultAppletSize.height;
        }

        container.applet = applet;
        //coordinated passed by param?
        if ( x >= 0 && y >= 0) {
            if (x + container.width > root.width) {
                x = root.width - container.width - 10;
            }
            if (y + container.height > root.height) {
                x = root.height - container.height;
            }
            container.x = x;
            container.y = y;
        //coordinates stored?
        } else if (config !== undefined && config.x !== undefined && config.y !== undefined &&
            config.x >= 0 && config.y >= 0) {
            container.x = config.x;
            container.y = config.y;
        }

        //rotation sotired and significative?
        if (config !== undefined && config.rotation !== undefined &&
            (config.rotation > 5 || config.rotation < -5)) {
            container.rotation = config.rotation;
        }

        LayoutManager.itemGroups[container.category] = container;
//         print("Applet " + container.category + applet.title + " added at" + container.x + " " + container.y);

        if (container.x >= 0 && container.y >= 0) {
            LayoutManager.positionItem(container);
        }
    }

    function addLauncher(desktopUrl) {
        itemView.linkHere(desktopUrl);
    }

    onDragEnter: {
        if (isContainment && !event.mimeData.urls.length) {
            placeHolder.width = LayoutManager.defaultAppletSize.width;
            placeHolder.height = LayoutManager.defaultAppletSize.height;
            placeHolder.x = event.x - placeHolder.width / 2;
            placeHolder.y = event.y - placeHolder.width / 2;
            LayoutManager.positionItem(placeHolder);
            LayoutManager.setSpaceAvailable(placeHolder.x, placeHolder.y, placeHolder.width, placeHolder.height, true);
            placeHolderPaint.opacity = root.haloOpacity;
        }
    }

    onDragMove: {
        // TODO: We should reject drag moves onto file items that don't accept drops
        // (cf. QAbstractItemModel::flags() here, but DeclarativeDropArea currently
        // is currently incapable of rejecting drag events.

        // Trigger autoscroll.
        if (event.mimeData.urls.length) {
            itemView.scrollLeft = (event.x < (units.largeSpacing * 3));
            itemView.scrollRight = (event.x > width - (units.largeSpacing * 3));
            itemView.scrollUp = (event.y < (units.largeSpacing * 3));
            itemView.scrollDown = (event.y > height - (units.largeSpacing * 3));
        }

        if (isContainment && !event.mimeData.urls.length) {
            placeHolder.width = LayoutManager.defaultAppletSize.width;
            placeHolder.height = LayoutManager.defaultAppletSize.height;
            placeHolder.x = event.x - placeHolder.width / 2;
            placeHolder.y = event.y - placeHolder.width / 2;
            LayoutManager.positionItem(placeHolder);
            LayoutManager.setSpaceAvailable(placeHolder.x, placeHolder.y, placeHolder.width, placeHolder.height, true);
            placeHolderPaint.opacity = root.haloOpacity;
        }
    }

    onDragLeave: {
        // Cancel autoscroll.
        if (event.mimeData.urls.length) {
            itemView.scrollLeft = false;
            itemView.scrollRight = false;
            itemView.scrollUp = false;
            itemView.scrollDown = false;
        }

        if (isContainment) {
            placeHolderPaint.opacity = 0;
        }
    }

    onDrop: {
        // Cancel autoscroll.
        if (event.mimeData.urls.length) {
            itemView.scrollLeft = false;
            itemView.scrollRight = false;
            itemView.scrollUp = false;
            itemView.scrollDown = false;
        }

        if (event.mimeData.urls.length) {
            itemView.drop(root, event, mapToItem(itemView, event.x, event.y));
        }

        if (isContainment && !event.mimeData.urls.length) {
            placeHolderPaint.opacity = 0;
            plasmoid.processMimeData(event.mimeData, event.x - placeHolder.width / 2, event.y - placeHolder.height / 2);
        }
    }

    Connections {
        target: plasmoid

        ignoreUnknownSignals: true

        onAppletAdded: {
            addApplet(applet, x, y);
            //clean any eventual invalid chunks in the config
            LayoutManager.save();
        }

        onAppletRemoved: {
            //clean any eventual invalid chunks in the config
//             console.log("Applet removed");
            LayoutManager.removeApplet(applet);
            LayoutManager.save();
        }
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

    PlasmaCore.Svg {
        id: actionOverlays

        imagePath: "widgets/action-overlays"
        multipleImages: true
        size: "16x16"
    }

    Folder.SystemSettings {
        id: systemSettings
    }

    Folder.MenuHelper {
        id: menuHelper
    }

    Folder.ViewPropertiesMenu {
        id: viewPropertiesMenu

        onArrangementChanged: {
            plasmoid.configuration.arrangement = arrangement;
        }

        onAlignmentChanged: {
            plasmoid.configuration.alignment = alignment;
        }

        onLockedChanged: {
            plasmoid.configuration.locked = locked;
        }

        onSortModeChanged: {
            plasmoid.configuration.sortMode = sortMode;
        }

        onSortDescChanged: {
            plasmoid.configuration.sortDesc = sortDesc;
        }

        onSortDirsFirstChanged: {
            plasmoid.configuration.sortDirsFirst = sortDirsFirst;
        }

        Component.onCompleted: {
            arrangement = plasmoid.configuration.arrangement;
            alignment = plasmoid.configuration.alignment;
            locked = plasmoid.configuration.locked;
            sortMode = plasmoid.configuration.sortMode;
            sortDesc = plasmoid.configuration.sortDesc;
            sortDirsFirst = plasmoid.configuration.sortDirsFirst;
        }
    }

    PlasmaComponents.Label {
        anchors.fill: parent

        text: itemView.errorString

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
    }

    ToolTipDelegate {
        id: toolTipDelegate

        visible: false
    }

    Connections {
        target: plasmoid.configuration

        onArrangementChanged: {
            viewPropertiesMenu.arrangement = plasmoid.configuration.arrangement;
        }

        onAlignmentChanged: {
            viewPropertiesMenu.alignment = plasmoid.configuration.alignment;
        }

        onLockedChanged: {
            viewPropertiesMenu.locked = plasmoid.configuration.locked;
        }

        onSortModeChanged: {
            itemView.sortMode = plasmoid.configuration.sortMode;
            viewPropertiesMenu.sortMode = plasmoid.configuration.sortMode;
        }

        onSortDescChanged: {
            viewPropertiesMenu.sortDesc = plasmoid.configuration.sortDesc;
        }

        onSortDirsFirstChanged: {
            viewPropertiesMenu.sortDirsFirst = plasmoid.configuration.sortDirsFirst;
        }

        onPositionsChanged: {
            itemView.positions = plasmoid.configuration.positions;
        }
    }

    ItemView {
        id: itemView

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: root.label != null ? root.label.height : 0
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        isRootView: true

        url: plasmoid.configuration.url
        locked: (plasmoid.configuration.locked || !isContainment)
        filterMode: plasmoid.configuration.filterMode
        filterPattern: plasmoid.configuration.filterPattern
        filterMimeTypes: plasmoid.configuration.filterMimeTypes

        flow: (plasmoid.configuration.arrangement == 0) ? GridView.FlowLeftToRight : GridView.FlowTopToBottom
        layoutDirection: (plasmoid.configuration.alignment == 0) ? Qt.LeftToRight : Qt.RightToLeft

        onSortModeChanged: {
            plasmoid.configuration.sortMode = sortMode;
        }

        onPositionsChanged: {
            plasmoid.configuration.positions = itemView.positions;
        }

        Component.onCompleted: {
            itemView.sortMode = plasmoid.configuration.sortMode;
            itemView.positions = plasmoid.configuration.positions;
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

        //This is just for event compression when a lot of boxes are created one after the other
        Timer {
            id: layoutTimer
            repeat: false
            running: false
            interval: 100
            onTriggered: {
                LayoutManager.resetPositions()
                for (var i=0; i<resultsFlow.children.length; ++i) {
                    var child = resultsFlow.children[i]
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

            x: -10000 // move offscreen initially to avoid flickering
            width: 100
            height: 100

            property bool animationsEnabled
            property int minimumWidth
            property int minimumHeight
            property Item syncItem

            function syncWithItem(item) {
                syncItem = item
                minimumWidth = item.minimumWidth
                minimumHeight = item.minimumHeight
                repositionTimer.running = true
                if (placeHolderPaint.opacity < 1) {
                    placeHolder.delayedSyncWithItem()
                }
            }

            function delayedSyncWithItem() {
                placeHolder.x = placeHolder.syncItem.x
                placeHolder.y = placeHolder.syncItem.y
                placeHolder.width = placeHolder.syncItem.width
                placeHolder.height = placeHolder.syncItem.height
                //only positionItem here, we don't want to save
                LayoutManager.positionItem(placeHolder)
                LayoutManager.setSpaceAvailable(placeHolder.x, placeHolder.y, placeHolder.width, placeHolder.height, true)
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

            x: placeHolder.x + (root.iconSize/2)
            y: placeHolder.y + (root.iconSize/2)
            width: placeHolder.width + (root.iconSize/2)
            height: placeHolder.height - root.iconSize
            z: 0
            visible: false

            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    Component {
        id: labelComponent

        PlasmaComponents.Label {
            width: parent.width
            height: visible ? implicitHeight : 0

            visible: (plasmoid.configuration.labelMode != 0)

            horizontalAlignment: Text.AlignHCenter
            text: labelGenerator.displayLabel

            Folder.LabelGenerator {
                id: labelGenerator

                url: plasmoid.configuration.url
                rtl: (Qt.application.layoutDirection == Qt.RightToLeft)
                labelMode: plasmoid.configuration.labelMode
                labelText: plasmoid.configuration.labelText
            }
        }
    }

    Component.onCompleted: {
        var actionName = "";
        var modelAction = null;

        for (var i = 0; i < sharedActions.length; i++) {
            actionName = sharedActions[i];
            modelAction = itemView.model.action(actionName);
            plasmoid.setAction(actionName, modelAction.text, menuHelper.iconName(modelAction));

            if (actionName == "newMenu") {
                menuHelper.setMenu(plasmoid.action(actionName), itemView.model.newMenu);
                plasmoid.setActionSeparator("separator1");

                plasmoid.setAction("viewProperties", i18n("Icons"), "preferences-desktop-icons");
                menuHelper.setMenu(plasmoid.action("viewProperties"), viewPropertiesMenu.menu);
            } else {
                plasmoid.action(actionName).triggered.connect(modelAction.trigger);
            }
        }

        plasmoid.setActionSeparator("separator2");

        Plasmoid.contextualActionsAboutToShow.connect(root.updateContextualActions);

        if (!isContainment) {
            root.label = labelComponent.createObject(root);

            return;
        } else {
            //WORKAROUND: that's the only place where we can inject a sensible size.
            //if root has width defined, it will override the value we set before
            //the component completes
            root.width = plasmoid.width
        }

        placeHolderPaint.opacity = 0;
        placeHolderPaint.visible = true;
//         print("Containment completed.");
        LayoutManager.resultsFlow = resultsFlow
        LayoutManager.plasmoid = plasmoid
        updateGridSize()
        //plasmoid.containmentType = "DesktopContainment"

        LayoutManager.restore()

        for (var i = 0; i < plasmoid.applets.length; ++i) {
            var applet = plasmoid.applets[i]
            addApplet(applet, -1, -1);
        }
        //clean any eventual invalid chunks in the config
        LayoutManager.save();
    }
}
