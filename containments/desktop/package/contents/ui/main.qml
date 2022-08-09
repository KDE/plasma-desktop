/*
    SPDX-FileCopyrightText: 2011-2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2011-2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.draganddrop 2.0 as DragDrop
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

import org.kde.private.desktopcontainment.desktop 0.1 as Desktop
import org.kde.private.desktopcontainment.folder 0.1 as Folder

import org.kde.plasma.private.containmentlayoutmanager 1.0 as ContainmentLayoutManager

import "code/FolderTools.js" as FolderTools

FolderViewDropArea {
    id: root
    objectName: isFolder ? "folder" : "desktop"

    width: isPopup ? undefined : preferredWidth(false) // Initial size when adding to e.g. desktop.
    height: isPopup ? undefined : preferredHeight(false) // Initial size when adding to e.g. desktop.

    Layout.minimumWidth: preferredWidth(!isPopup)
    Layout.minimumHeight: preferredHeight(!isPopup)

    Layout.preferredWidth: preferredWidth(false)
    Layout.preferredHeight: preferredHeight(false)

    Layout.maximumWidth: isPopup ? preferredWidth(false) : -1
    Layout.maximumHeight: isPopup ? preferredHeight(false) : -1

    function switchSize() {
        // Support expanding into the full representation on very thick vertical panels.
        if (isPopup && plasmoid.formFactor === PlasmaCore.Types.Vertical) {
            return PlasmaCore.Units.gridUnit * 8;
        }

        return 0;
    }

    Plasmoid.switchWidth: { switchSize(); }
    Plasmoid.switchHeight: { switchSize(); }

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    property bool isFolder: (plasmoid.pluginName === "org.kde.plasma.folder")
    property bool isContainment: ("containmentType" in plasmoid)
    property bool isPopup: (plasmoid.location !== PlasmaCore.Types.Floating)
    property bool useListViewMode: isPopup && plasmoid.configuration.viewMode === 0

    property Component appletAppearanceComponent
    property Item toolBox

    property int handleDelay: 800
    property real haloOpacity: 0.5

    property int iconSize: PlasmaCore.Units.iconSizes.small
    property int iconWidth: iconSize
    property int iconHeight: iconWidth

    readonly property int hoverActivateDelay: 750 // Magic number that matches Dolphin's auto-expand folders delay.

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

        rightMargin: (isContainment && plasmoid.availableScreenRect && parent)
            ? (parent.width - plasmoid.availableScreenRect.x - plasmoid.availableScreenRect.width) : 0

        bottomMargin: (isContainment && plasmoid.availableScreenRect && parent)
            ? (parent.height - plasmoid.availableScreenRect.y - plasmoid.availableScreenRect.height) : 0
    }

    Behavior on anchors.topMargin {
        NumberAnimation { duration: PlasmaCore.Units.longDuration; easing.type: Easing.InOutQuad }
    }
    Behavior on anchors.leftMargin {
        NumberAnimation { duration: PlasmaCore.Units.longDuration; easing.type: Easing.InOutQuad }
    }
    Behavior on anchors.rightMargin {
        NumberAnimation { duration: PlasmaCore.Units.longDuration; easing.type: Easing.InOutQuad }
    }
    Behavior on anchors.bottomMargin {
        NumberAnimation { duration: PlasmaCore.Units.longDuration; easing.type: Easing.InOutQuad }
    }

    function updateGridSize() {
        appletsLayout.cellWidth = root.iconWidth + toolBoxSvg.elementSize("left").width + toolBoxSvg.elementSize("right").width;
        appletsLayout.cellHeight = root.iconHeight + toolBoxSvg.elementSize("top").height + toolBoxSvg.elementSize("bottom").height;
        appletsLayout.defaultItemWidth = appletsLayout.cellWidth * 6;
        appletsLayout.defaultItemHeight = appletsLayout.cellHeight * 6;
    }

    function addLauncher(desktopUrl) {
        if (!isFolder) {
            return;
        }

        folderViewLayer.view.linkHere(desktopUrl);
    }

    function preferredWidth(minimum) {
        if (isContainment || !folderViewLayer.ready) {
            return -1;
        } else if (useListViewMode) {
            return (minimum ? folderViewLayer.view.cellHeight * 4 : PlasmaCore.Units.gridUnit * 16);
        }

        return (folderViewLayer.view.cellWidth * (minimum ? 1 : 3)) + (PlasmaCore.Units.largeSpacing * 2);
    }

    function preferredHeight(minimum) {
        let height;
        if (isContainment || !folderViewLayer.ready) {
            return -1;
        } else if (useListViewMode) {
            height = (folderViewLayer.view.cellHeight * (minimum ? 1 : 15)) + PlasmaCore.Units.smallSpacing;
        } else {
            height = (folderViewLayer.view.cellHeight * (minimum ? 1 : 2)) + PlasmaCore.Units.largeSpacing;
        }

        if (plasmoid.configuration.labelMode !== 0) {
            height += folderViewLayer.item.labelHeight;
        }

        return height;
    }

    function isDrag(fromX, fromY, toX, toY) {
        const length = Math.abs(fromX - toX) + Math.abs(fromY - toY);
        return length >= Qt.styleHints.startDragDistance;
    }

    onFocusChanged: {
        if (focus && isFolder) {
            folderViewLayer.item.forceActiveFocus();
        }
    }

    onDragEnter: {
        if (isContainment && plasmoid.immutable && !(isFolder && FolderTools.isFileDrag(event))) {
            event.ignore();
        }

        // Don't allow any drops while listing.
        if (isFolder && folderViewLayer.view.status === Folder.FolderModel.Listing) {
            event.ignore();
        }

        // Firefox tabs are regular drags. Since all of our drop handling is asynchronous
        // we would accept this drop and have Firefox not spawn a new window. (Bug 337711)
        if (event.mimeData.formats.indexOf("application/x-moz-tabbrowser-tab") !== -1) {
            event.ignore();
        }
    }

    onDragMove: {
        // TODO: We should reject drag moves onto file items that don't accept drops
        // (cf. QAbstractItemModel::flags() here, but DeclarativeDropArea currently
        // is currently incapable of rejecting drag events.

        // Trigger autoscroll.
        if (isFolder && FolderTools.isFileDrag(event)) {
            handleDragMove(folderViewLayer.view, mapToItem(folderViewLayer.view, event.x, event.y));
        } else if (isContainment) {
            appletsLayout.showPlaceHolderAt(
                Qt.rect(event.x - appletsLayout.minimumItemWidth / 2,
                event.y - appletsLayout.minimumItemHeight / 2,
                appletsLayout.minimumItemWidth,
                appletsLayout.minimumItemHeight)
            );
        }
    }

    onDragLeave: {
        // Cancel autoscroll.
        if (isFolder) {
            handleDragEnd(folderViewLayer.view);
        }

        if (isContainment) {
            appletsLayout.hidePlaceHolder();
        }
    }

    onDrop: {
        if (isFolder && FolderTools.isFileDrag(event)) {
            handleDragEnd(folderViewLayer.view);
            folderViewLayer.view.drop(root, event, mapToItem(folderViewLayer.view, event.x, event.y));
        } else if (isContainment) {
            plasmoid.processMimeData(event.mimeData,
                        event.x - appletsLayout.placeHolder.width / 2, event.y - appletsLayout.placeHolder.height / 2);
            event.accept(event.proposedAction);
            appletsLayout.hidePlaceHolder();
        }
    }

    Component {
        id: compactRepresentation
        CompactRepresentation { folderView: folderViewLayer.view }
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

    // Can be removed?
    KQuickControlsAddons.EventGenerator {
        id: eventGenerator
    }

    Connections {
        target: plasmoid
        ignoreUnknownSignals: true
        function onEditModeChanged() {
            appletsLayout.editMode = plasmoid.editMode;
        }
    }

    ContainmentLayoutManager.AppletsLayout {
        id: appletsLayout
        anchors.fill: parent
        relayoutLock: width != plasmoid.availableScreenRect.width || height != plasmoid.availableScreenRect.height
        // NOTE: use plasmoid.availableScreenRect and not own width and height as they are updated not atomically
        configKey: "ItemGeometries-" + Math.round(plasmoid.screenGeometry.width) + "x" + Math.round(plasmoid.screenGeometry.height)
        fallbackConfigKey: plasmoid.availableScreenRect.width > plasmoid.availableScreenRect.height ? "ItemGeometriesHorizontal" : "ItemGeometriesVertical"

        containment: plasmoid
        editModeCondition: plasmoid.immutable
                ? ContainmentLayoutManager.AppletsLayout.Locked
                : ContainmentLayoutManager.AppletsLayout.AfterPressAndHold

        // Sets the containment in edit mode when we go in edit mode as well
        onEditModeChanged: plasmoid.editMode = editMode;

        minimumItemWidth: PlasmaCore.Units.gridUnit * 3
        minimumItemHeight: minimumItemWidth

        cellWidth: PlasmaCore.Units.iconSizes.small
        cellHeight: cellWidth

        eventManagerToFilter: folderViewLayer.item ? folderViewLayer.item.view.view : null

        appletContainerComponent: ContainmentLayoutManager.BasicAppletContainer {
            id: appletContainer
            editModeCondition: plasmoid.immutable
                ? ContainmentLayoutManager.ItemContainer.Locked
                : ContainmentLayoutManager.ItemContainer.AfterPressAndHold
            configOverlayComponent: ConfigOverlay {}
            onUserDrag: {
                var pos = mapToItem(root.parent, dragCenter.x, dragCenter.y);
                var newCont = plasmoid.containmentAt(pos.x, pos.y);

                if (newCont && newCont !== plasmoid) {
                    var newPos = newCont.mapFromApplet(plasmoid, pos.x, pos.y);

                    newCont.addApplet(appletContainer.applet, newPos.x, newPos.y);
                    appletsLayout.hidePlaceHolder();
                }
            }
            component DropAnimation : NumberAnimation {
                duration: PlasmaCore.Units.shortDuration
                easing.type: Easing.InOutQuad
            }
            Behavior on x {
                DropAnimation { }
            }
            Behavior on y {
                DropAnimation { }
            }
        }

        placeHolder: ContainmentLayoutManager.PlaceHolder {}

        Loader {
            id: folderViewLayer

            anchors.fill: parent

            property bool ready: status === Loader.Ready
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

                // `FolderViewDropArea` is not a FocusScope. We need to forward manually.
                function onPressed() {
                    folderViewLayer.forceActiveFocus();
                }
            }
        }
    }

    Component.onCompleted: {
        if (!isContainment) {
            return;
        }

        // Customize the icon and text to improve discoverability
        plasmoid.setAction("configure", i18n("Configure Desktop and Wallpaper…"), "preferences-desktop-wallpaper")

        // WORKAROUND: that's the only place where we can inject a sensible size.
        // if root has width defined, it will override the value we set before
        // the component completes
        root.width = plasmoid.width;

        updateGridSize();
    }
}
