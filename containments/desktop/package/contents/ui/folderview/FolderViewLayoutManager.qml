/*
    SPDX-FileCopyrightText: 2026 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg as KSvg
import org.kde.config // for KAuthorized
import org.kde.kirigami as Kirigami
import org.kde.kquickcontrolsaddons

import org.kde.private.desktopcontainment.folder as Folder

import org.kde.plasma.private.containmentlayoutmanager as ContainmentLayoutManager

import "../code/FolderTools.js" as FolderTools

ContainmentLayoutManager.AppletsLayout {
    id: main

    property alias url: dir.url
    property real iconSize: FolderTools.iconSizeFromTheme(Plasmoid.configuration.iconSize)
    property FolderItemDelegate hoveredItem: null
    property FolderItemDelegate pressedItem: null
    readonly property bool useListViewMode: false
    property RenameEditor editor: null
    readonly property int rows: height / cellHeight
    readonly property int columns: width / cellWidth

    property Item rubberBand: null
    property var cachedRectangleSelection: null
    property bool ctrlPressed: false

    property var draggingItems: []
    property var sharedActions: ["newMenu", "paste", "undo", "emptyTrash"]
    readonly property bool lockedByKiosk: !KAuthorized.authorize("editable_desktop_icons")

    // The graphical item of the containment this view is in, if any.
    containmentItem: {
        for (let item = parent; item; item = item.parent) {
            const layout = item as ContainmentLayoutManager.AppletsLayout;
            if (layout) {
                return layout.containmentItem;
            }
        }
        return null;
    }

    cellWidth: {
        const iconWidth = iconSize + (2 * Kirigami.Units.gridUnit) + (2 * Kirigami.Units.smallSpacing);
        if (false&&root.isContainment && isRootView && scrollArea.viewportWidth > 0) {
            const minIconWidth = Math.max(iconWidth, Kirigami.Units.iconSizes.small * ((Plasmoid.configuration.labelWidth * 2) + 4));
            const extraWidth = calcExtraSpacing(minIconWidth, scrollArea.viewportWidth);
            return minIconWidth + extraWidth;
        } else {
            return iconWidth;
        }
    }

    cellHeight: {
        // the smallSpacings are for padding
        const iconHeight = iconSize + (Kirigami.Units.gridUnit * Plasmoid.configuration.textLines) + (Kirigami.Units.smallSpacing * 3);
        if (false&&root.isContainment && isRootView && scrollArea.viewportHeight > 0) {
            let extraHeight = calcExtraSpacing(iconHeight, scrollArea.viewportHeight);
            return iconHeight + extraHeight;
        } else {
            return iconHeight;
        }
    }

    function updateContextualActions() {
        dir.updateActions();

        for (let i = 0, len = sharedActions.length; i < len; i++) {
            const actionName = sharedActions[i];
            const appletAction = Plasmoid.internalAction(actionName);
            if (appletAction) {
                const modelAction = dir.action(actionName);
                appletAction.text = modelAction.text;
                appletAction.enabled = modelAction.enabled;
                appletAction.visible = modelAction.visible;
            }
        }
    }

    function selectedItem() {
        for (let i = 0; i < repeater.count; i++) {
            const item = repeater.itemAt(i) as FolderItemDelegate;
            if (item && !item.blank && dir.isSelected(item.index)) {
                return item;
            }
        }
        return null;
    }

    function rename() {
        let renameAction = dir.action("rename");
        if (renameAction && !renameAction.enabled) {
            return;
        }

        const target = selectedItem();
        if (!target) {
            return;
        }

        if (!main.editor) {
            main.editor = editorComponent.createObject(main);
        }

        main.editor.targetItem = target;
        print("HHH",main.editor.targetItem );
    }

    function cancelRename() {
        if (main.editor) {
            main.editor.targetItem = null;
        }
    }

    function dropItemAt(pos) {
        let item = childAt(pos.x, pos.y);

        if (item && item?.isDir && !item?.blank) {
            //gridView.currentIndex = item.index
            item.forceActiveFocus();
            return item.index;
        }

        return -1;
    }

    function drop(target, event, pos) {
        let dropPos = target.mapToItem(this, pos.x, pos.y);
        dir.drop(target, event, dropItemAt(dropPos), Plasmoid.isContainment && !Plasmoid.immutable);
    }

    function isDrag(fromX, fromY, toX, toY) {
        const length = Math.abs(fromX - toX) + Math.abs(fromY - toY);
        return length >= Application.styleHints.startDragDistance;
    }

    function rectangleSelect(x, y, width, height, rubberBand) {
        const indices = [];

        for (let i = 0; i < repeater.count; i++) {
            const item = repeater.itemAt(i) as FolderItemDelegate;

            if (!item || item.blank) {
                continue;
            }

            if (rubberBand.intersects(Qt.rect(item.x, item.y, item.width, item.height))) {
                indices.push(i);
            }
        }

        main.cachedRectangleSelection = indices;
    }

    onCachedRectangleSelectionChanged: {
        if (cachedRectangleSelection === null) {
            return;
        }

        dir.updateSelection(cachedRectangleSelection, main.ctrlPressed);
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Menu) {
            const target = main.selectedItem();
            if (target) {
                dir.openContextMenu(target, event.modifiers, false);
                event.accepted = true;
            }
        }
    }

    function generateDragImage() {
        for (let i = 0; i < repeater.count; i++) {
            let item = repeater.itemAt(i) as FolderItemDelegate;
            if (item) {
                item.updateDragImage();
            }
        }
    }

    PlasmaCore.Action {
        id: viewPropertiesAction
        text: i18nc("@item:inmenu opens submenu with view options like sorting", "Icons")
        icon.name: "view-list-icons"
        menu: viewPropertiesMenu.menu
    }
    PlasmaCore.Action {
        id: actionSeparator
        isSeparator: true
    }
    Component.onCompleted: {
        for (let i = 0, len = sharedActions.length; i < len; i++) {
            const actionName = sharedActions[i];
            const modelAction = dir.action(actionName);
            Plasmoid.contextualActions.push(modelAction)
            if (actionName === "emptyTrash") {
                Plasmoid.contextualActions.push(viewPropertiesAction)
            }
        }

        Plasmoid.contextualActions.push(actionSeparator);

        Plasmoid.contextualActionsAboutToShow.connect(updateContextualActions);
        Plasmoid.contextualActionsAboutToShow.connect(dir.clearSelection);
    }
    Folder.ViewPropertiesMenu {
        id: viewPropertiesMenu

        showLayoutActions: !root.inPanel
        showLockAction: Plasmoid.isContainment
        showIconSizeActions: !main.useListViewMode

        lockedEnabled: !main.lockedByKiosk

        onArrangementChanged: {
            Plasmoid.configuration.arrangement = arrangement;
        }

        onAlignmentChanged: {
            Plasmoid.configuration.alignment = alignment;
        }

        onPreviewsChanged: {
            Plasmoid.configuration.previews = previews;
        }

        onLockedChanged: {
            if (!main.lockedByKiosk) {
                Plasmoid.configuration.locked = locked;
            }
        }

        onSortModeChanged: {
            Plasmoid.configuration.sortMode = sortMode;
        }

        onSortDescChanged: {
            Plasmoid.configuration.sortDesc = sortDesc;
        }

        onSortDirsFirstChanged: {
            Plasmoid.configuration.sortDirsFirst = sortDirsFirst;
        }

        onIconSizeChanged: {
            Plasmoid.configuration.iconSize = iconSize;
        }

        Component.onCompleted: {
            arrangement = Plasmoid.configuration.arrangement;
            alignment = Plasmoid.configuration.alignment;
            previews = Plasmoid.configuration.previews;
            locked = Plasmoid.configuration.locked || main.lockedByKiosk;
            sortMode = Plasmoid.configuration.sortMode;
            sortDesc = Plasmoid.configuration.sortDesc;
            sortDirsFirst = Plasmoid.configuration.sortDirsFirst;
            iconSize = Plasmoid.configuration.iconSize;
        }
    }

    Folder.EventGenerator {
        id: eventGenerator
    }

    Folder.ShortCut {
        Component.onCompleted: {
            installAsEventFilterFor(main);
        }

        onDeleteFile: {
            dir.deleteSelected();
        }

        onRenameFile: {
            main.rename();
        }

        onMoveToTrash: {
            const action = dir.action("trash");
            if (action && action.enabled) {
                action.trigger();
            }
        }

        onCreateFolder: {
            dir.createFolder();
        }
    }

    MouseArea {
        anchors.fill: parent
        property point pressPos
        enabled: !Plasmoid.containment.corona.editMode && !root.editMode
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        function pressCanceled() {
            if (main.rubberBand) {
                main.rubberBand.close();
                main.rubberBand = null;

                main.cachedRectangleSelection = null;
                dir.unpinSelection();
            }
        }

        onPressed: (mouse) => pressPos = Qt.point(mouse.x, mouse.y)
        onCanceled: pressCanceled()
        onReleased: pressCanceled()
        onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton && !main.isDrag(pressPos.x, pressPos.y, mouse.x, mouse.y)) {
                main.cancelRename();
                dir.clearSelection();
            }
            eventGenerator.sendMouseEvent(root, Folder.EventGenerator.MouseButtonPress, mouse.x, mouse.y, mouse.button, mouse.buttons, mouse.modifiers);
        }
        onPositionChanged: (mouse) => {
            if (main.rubberBand) {
                main.rubberBand.x = Math.min(pressPos.x, mouse.x);
                main.rubberBand.y = Math.min(pressPos.y, mouse.y);
                main.rubberBand.width = Math.max(1, Math.max(pressPos.x, mouse.x) - main.rubberBand.x);
                main.rubberBand.height = Math.max(1, Math.max(pressPos.y, mouse.y) - main.rubberBand.y);
                Qt.callLater(main.rectangleSelect, rubberBand.x, rubberBand.y, rubberBand.width, rubberBand.height, main.rubberBand);
            } else if (main.isDrag(pressPos.x, pressPos.y, mouse.x, mouse.y)) {
                // Disable rubberband while renaming
                if (main.editor && main.editor.targetItem) {
                    return;
                }
                dir.pinSelection();
                main.rubberBand = rubberBandObject.createObject(main, {x: pressPos.x, y: pressPos.y});
            }
        }
    }

    Component {
        id: rubberBandObject

        Rectangle {
            id: rubberBand

            width: 0
            height: 0
            z: 99999

            SystemPalette {
                id: sysPalette
                colorGroup: SystemPalette.Active
            }

            radius: Kirigami.Units.cornerRadius
            border.color: sysPalette.highlight
            color: Qt.alpha(border.color, 0.3)

            function intersects(rect) {
                return x + width >= rect.x && y + height >= rect.y && rect.x + rect.width >= x && rect.y + rect.height >= y;
            }

            function close() {
                opacityAnimation.restart();
            }

            OpacityAnimator {
                id: opacityAnimation
                target: rubberBand
                to: 0
                from: 1
                duration: Kirigami.Units.shortDuration

                // This easing curve has an elognated start, which works
                // better than a standard easing curve for the rubberband
                // animation, which fades out fast and is generally of a
                // small area.
                easing {
                    bezierCurve: [0.4, 0.0, 1, 1]
                    type: Easing.Bezier
                }

                onFinished: {
                    rubberBand.visible = false;
                    rubberBand.enabled = false;
                    // We need to explicitly generate an image here
                    // to make sure we have one before we start dragging
                    main.generateDragImage();
                    rubberBand.destroy();
                }
            }
        }
    }


    Folder.ItemViewAdapter {
        id: viewAdapter

        adapterView: main
        adapterModel: positioner
        adapterIconSize: main.iconSize * 2
        adapterVisibleArea: Qt.rect(0, 0, main.width, main.height)

        Component.onCompleted: {
           // gridView.movementStarted.connect(viewAdapter.viewScrolled);
            dir.viewAdapter = viewAdapter;
        }
    }
    Repeater {
        id: repeater
        model: Folder.FolderModel {
            id: dir

            url: Plasmoid.configuration.url
            usedByContainment: true//Plasmoid.isContainment
            sortDesc: Plasmoid.configuration.sortDesc
            sortDirsFirst: Plasmoid.configuration.sortDirsFirst
            parseDesktopFiles: (Plasmoid.configuration.url === "desktop:/")
            previews: Plasmoid.configuration.previews
            previewPlugins: Plasmoid.configuration.previewPlugins
            applet: Plasmoid

           onRequestRename: main.rename()

            onPopupMenuAboutToShow: (dropJob, mimeData, x, y) => {
                if (Plasmoid.isContainment && !Plasmoid.immutable) {
                    root.processMimeData(mimeData, x, y, dropJob);
                }
            }

            // Create drag images before dragging
            // Due to async operations we can't call this before dragging starts,
            // but we have to call it after a selection is done
            onSelectionDone: main.generateDragImage()


            onMove: (x, y, urls) => {
                print("MOVE",x,y,urls)

                main.generateDragImage(); // in case the user drags again
            }
        }

        delegate: FolderItemDelegate {
            id: delegate

            isOnRootView: true

            Component.onCompleted: {
                main.positionItem(this);
            }

            width: main.cellWidth
            height: main.cellHeight

            preferredLayoutDirection: ContainmentLayoutManager.AppletsLayout.LeftToRight // TODO: all folderview layout preferences and RTL mode
        }
    }

    Component {
        id: editorComponent

        RenameEditor {
            id: editor

            view: main
            model: dir

            visible: false
            z: 100000

            onCommit: {
                if (targetItem) {
                    dir.rename(targetItem.index, text);
                    targetItem = null;
                    main.forceActiveFocus();
                }
            }

            onVisibleChanged: {
                if (visible) {
                    focus = true;
                } else {
                    main.forceActiveFocus();
                }
            }
        }
    }
}
