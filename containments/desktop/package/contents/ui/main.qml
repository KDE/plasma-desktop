/*
    SPDX-FileCopyrightText: 2011-2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2011-2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg as KSvg
import org.kde.kquickcontrolsaddons as KQuickControlsAddons
import org.kde.kirigami as Kirigami

import org.kde.private.desktopcontainment.folder as Folder

import org.kde.plasma.private.containmentlayoutmanager as ContainmentLayoutManager

import "code/FolderTools.js" as FolderTools

ContainmentItem {
    id: root

    switchWidth: { switchSize(); }
    switchHeight: { switchSize(); }

    // Only exists because the default CompactRepresentation doesn't:
    // - open on drag
    // - allow defining a custom drop handler
    // TODO remove once it gains that feature (perhaps optionally?)
    compactRepresentation: (isFolder && !isContainment) ? compactRepresentation : null

    objectName: isFolder ? "folder" : "desktop"

    width: isPopup ? undefined : preferredWidth(false) // Initial size when adding to e.g. desktop.
    height: isPopup ? undefined : preferredHeight(false) // Initial size when adding to e.g. desktop.

    function switchSize() {
        // Support expanding into the full representation on very thick vertical panels.
        if (isPopup && Plasmoid.formFactor === PlasmaCore.Types.Vertical) {
            return Kirigami.Units.gridUnit * 8;
        }

        return 0;
    }

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    property bool isFolder: (Plasmoid.pluginName === "org.kde.plasma.folder")
    property bool isContainment: Plasmoid.isContainment
    property bool isPopup: (Plasmoid.location !== PlasmaCore.Types.Floating)
    property bool useListViewMode: isPopup && Plasmoid.configuration.viewMode === 0

    property Component appletAppearanceComponent
    property Item toolBox

    property int handleDelay: 800
    property real haloOpacity: 0.5

    readonly property int screen: Plasmoid.containment.screen
    readonly property bool isUiReady: Plasmoid.containment.corona.isScreenUiReady(root.screen)

    readonly property int hoverActivateDelay: 750 // Magic number that matches Dolphin's auto-expand folders delay.

    readonly property Loader folderViewLayer: fullRepresentationItem.folderViewLayer
    readonly property ContainmentLayoutManager.AppletsLayout appletsLayout: fullRepresentationItem.appletsLayout

    // Plasmoid.title is set by a Binding {} in FolderViewLayer
    toolTipSubText: ""
    Plasmoid.icon: (!Plasmoid.configuration.useCustomIcon && folderViewLayer.ready) ? symbolicizeIconName(folderViewLayer.view?.model.iconName) : Plasmoid.configuration.icon

    // We want to do this here rather than in the model because we don't always want
    // symbolic icons everywhere, but we do know that we always want them in this
    // specific representation right here
    function symbolicizeIconName(iconName) {
        const symbolicSuffix = "-symbolic";
        if (iconName.endsWith(symbolicSuffix)) {
            return iconName;
        }

        return iconName + symbolicSuffix;
    }

    function addLauncher(desktopUrl) {
        if (!isFolder) {
            return;
        }

        folderViewLayer.view.linkHere(desktopUrl);
    }

    function preferredWidth(forMinimumSize: bool): real {
        if ((isContainment || !folderViewLayer.ready) || (isPopup && !compactRepresentationItem.visible)) {
            return -1;
        } else if (useListViewMode) {
            return (forMinimumSize ? folderViewLayer.view.cellHeight * 4 : Kirigami.Units.gridUnit * 16);
        }

        return (folderViewLayer.view.cellWidth * (forMinimumSize ? 1 : 3)) + (Kirigami.Units.gridUnit * 2);
    }

    function preferredHeight(forMinimumSize: bool): real {
        let height;
        if ((isContainment || !folderViewLayer.ready) || (isPopup && !compactRepresentationItem.visible)) {
            return -1;
        } else if (useListViewMode) {
            height = (folderViewLayer.view.cellHeight * (forMinimumSize ? 1 : 15)) + Kirigami.Units.smallSpacing;
        } else {
            height = (folderViewLayer.view.cellHeight * (forMinimumSize ? 1 : 2)) + Kirigami.Units.gridUnit;
        }

        if (Plasmoid.configuration.labelMode !== 0) {
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
            folderViewLayer.item?.forceActiveFocus();
        }
    }

    onExternalData: (mimetype, data) => {
        Plasmoid.configuration.url = data
    }

    component ShortDropBehavior : Behavior {
        NumberAnimation {
            duration: Kirigami.Units.shortDuration
            easing.type: Easing.InOutQuad
        }
    }

    component LongDropBehavior : Behavior {
        NumberAnimation {
            duration: Kirigami.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    KSvg.FrameSvgItem {
        id: highlightItemSvg

        visible: false

        imagePath: isPopup ? "widgets/viewitem" : ""
        prefix: "hover"
    }

    KSvg.FrameSvgItem {
        id: listItemSvg

        visible: false

        imagePath: isPopup ? "widgets/viewitem" : ""
        prefix: "normal"
    }

    KSvg.Svg {
        id: toolBoxSvg
        imagePath: "widgets/toolbox"
        property int rightBorder: elementSize("right").width
        property int topBorder: elementSize("top").height
        property int bottomBorder: elementSize("bottom").height
        property int leftBorder: elementSize("left").width
    }

    // FIXME: the use and existence of this property is a workaround
    preloadFullRepresentation: true
    fullRepresentation: FolderViewDropArea {
        id: dropArea

        anchors {
            fill: parent
            leftMargin: (isContainment && root.availableScreenRect) ? root.availableScreenRect.x : 0
            topMargin: (isContainment && root.availableScreenRect) ? root.availableScreenRect.y : 0

            rightMargin: (isContainment && root.availableScreenRect && parent)
                ? (parent.width - root.availableScreenRect.x - root.availableScreenRect.width) : 0

            bottomMargin: (isContainment && root.availableScreenRect && parent)
                ? (parent.height - root.availableScreenRect.y - root.availableScreenRect.height) : 0
        }

        LongDropBehavior on anchors.topMargin { }
        LongDropBehavior on anchors.leftMargin { }
        LongDropBehavior on anchors.rightMargin { }
        LongDropBehavior on anchors.bottomMargin { }

        property alias folderViewLayer: folderViewLayer
        property alias appletsLayout: appletsLayout

        // Layout size bindings are set in Component.onCompleted

        preventStealing: true

        onDragEnter: event => {
            if (isContainment && Plasmoid.immutable && !(isFolder && FolderTools.isFileDrag(event))) {
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

        onDragMove: event => {
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

        onDragLeave: event => {
            // Cancel autoscroll.
            if (isFolder) {
                handleDragEnd(folderViewLayer.view);
            }

            if (isContainment) {
                appletsLayout.hidePlaceHolder();
            }
        }

        onDrop: event => {
            if (isFolder && FolderTools.isFileDrag(event)) {
                handleDragEnd(folderViewLayer.view);
                folderViewLayer.view.drop(root, event, mapToItem(folderViewLayer.view, event.x, event.y));
            } else if (isContainment) {
                root.processMimeData(event.mimeData,
                    event.x - appletsLayout.placeHolder.width / 2,
                    event.y - appletsLayout.placeHolder.height / 2);
                event.accept(event.proposedAction);
                appletsLayout.hidePlaceHolder();
            }
        }

        Component {
            id: compactRepresentation
            CompactRepresentation { folderView: folderViewLayer.view }
        }

        Connections {
            target: Plasmoid.containment.corona
            ignoreUnknownSignals: true

            function onEditModeChanged() {
                appletsLayout.editMode = Plasmoid.containment.corona.editMode;
            }

            // When adding panels, sizes change. We want to make sure all panels
            // are loaded, and when they all are loaded, we tell the folderViewLayer loader to start.
            function onScreenUiReadyChanged(screen: int, newLayoutReady: bool) {
                if (root.isContainment && root.isFolder && !folderViewLayer.ready && Plasmoid.containment.screen === screen && newLayoutReady){
                    // We skip x and y since that is handled by the parent of folderViewLayer
                    folderViewLayer.active = true;
                }
            }
        }

        ContainmentLayoutManager.AppletsLayout {
            id: appletsLayout
            anchors.fill: parent
            relayoutLock: width !== root.availableScreenRect.width || height !== root.availableScreenRect.height
            // NOTE: use root.availableScreenRect and not own width and height as they are updated not atomically
            configKey: "ItemGeometries-" + Math.round(root.screenGeometry.width) + "x" + Math.round(root.screenGeometry.height)
            fallbackConfigKey: root.availableScreenRect.width > root.availableScreenRect.height ? "ItemGeometriesHorizontal" : "ItemGeometriesVertical"

            Binding on containment {
                value: Plasmoid
                when: Plasmoid.isContainment
            }
            containmentItem: root
            editModeCondition: Plasmoid.immutable
                    ? ContainmentLayoutManager.AppletsLayout.Locked
                    : ContainmentLayoutManager.AppletsLayout.AfterPressAndHold

            // Sets the containment in edit mode when we go in edit mode as well
            onEditModeChanged: Plasmoid.containment.corona.editMode = editMode;

            minimumItemWidth: Kirigami.Units.gridUnit * 3
            minimumItemHeight: minimumItemWidth

            cellWidth: Kirigami.Units.iconSizes.small
            cellHeight: cellWidth
            defaultItemWidth: cellWidth * 6
            defaultItemHeight: cellHeight * 6

            eventManagerToFilter: folderViewLayer.item?.view.view ?? null

            appletContainerComponent: ContainmentLayoutManager.BasicAppletContainer {
                id: appletContainer

                editModeCondition: Plasmoid.immutable
                    ? ContainmentLayoutManager.ItemContainer.Locked
                    : ContainmentLayoutManager.ItemContainer.AfterPressAndHold

                configOverlaySource: "ConfigOverlay.qml"

                onAppletChanged: {
                    applet.visible = true
                }

                Drag.dragType: Drag.Automatic
                Drag.active: false
                Drag.supportedActions: Qt.MoveAction
                Drag.mimeData: {
                    "text/x-plasmoidinstanceid": Plasmoid.containment.id+':'+appletContainer.applet.plasmoid.id
                }
                Drag.onDragFinished: dropEvent => {
                    if (dropEvent == Qt.MoveAction) {
                        appletContainer.visible = true
                        appletContainer.applet.visible = true
                        //currentApplet.applet.plasmoid.internalAction("remove").trigger()
                    } else {
                        appletContainer.visible = true
                        //appletsModel.insert(configurationArea.draggedItemIndex - 1, {applet: appletContainer.applet});
                    }
                    //appletContainer.destroy()
                    //root.dragAndDropping = false
                    //root.layoutManager.save()
                }

                onUserDrag: (newPosition, dragCenter) => {
                    const pos = mapToItem(root.parent, dragCenter.x, dragCenter.y);
                    const newCont = root.containmentItemAt(pos.x, pos.y);
                    // User likely touched screen edges, so ignore that.
                    if (!newCont) {
                        return;
                    }

                    if (newCont.plasmoid !== Plasmoid) {
                        // First go out of applet edit mode, get rid of the config overlay, release mouse grabs in preparation of applet reparenting
                        cancelEdit();
                        appletsLayout.hidePlaceHolder();
                        appletContainer.grabToImage(result => {
                            appletContainer.Drag.imageSource = result.url
                            appletContainer.visible = false
                            appletContainer.Drag.active = true
                        })
                    }
                }

                ShortDropBehavior on x { }
                ShortDropBehavior on y { }
            }

            placeHolder: ContainmentLayoutManager.PlaceHolder {}

            Loader {
                id: folderViewLayer

                anchors.fill: parent

                property bool ready: status === Loader.Ready
                property Item view: item?.view ?? null
                property QtObject model: item?.model ?? null

                focus: true

                // Do not set this active by default for desktop, and disable it when folderMode is not used
                active: {
                    if (root.isFolder){
                        if (!root.isContainment) {
                            // We are a folder widget
                            return true;
                        } else {
                            // For desktop, test if the screen is ready
                            return root.isUiReady;
                        }
                    }
                    return false;
                }
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

        PlasmaCore.Action {
            id: configAction
            text: i18n("Desktop and Wallpaper")
            icon.name: "preferences-desktop-wallpaper"
            shortcut:"Ctrl+Shift+D"
            onTriggered: Plasmoid.containment.configureRequested(Plasmoid)
        }

        Component.onCompleted: {
            // Layout bindings need to be set delayed; the intermediate steps as the other bindings happen cause loops
            Qt.callLater( () => {
                dropArea.Layout.minimumWidth = Qt.binding(() => preferredWidth(isPopup))
                dropArea.Layout.minimumHeight = Qt.binding(() => preferredHeight(isPopup))

                dropArea.Layout.preferredWidth = Qt.binding(() => preferredWidth(false))
                dropArea.Layout.preferredHeight = Qt.binding(() => preferredHeight(false))

                // Maximum size is intentionally unbounded
            })

            if (!Plasmoid.isContainment) {
                return;
            }

            Plasmoid.setInternalAction("configure", configAction)
        }
    }
}
