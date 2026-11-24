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
import org.kde.kirigami as Kirigami
import org.kde.kquickcontrolsaddons

import org.kde.private.desktopcontainment.folder as Folder

import org.kde.plasma.private.containmentlayoutmanager as ContainmentLayoutManager

import "../code/FolderTools.js" as FolderTools
import ".." as Base

ContainmentLayoutManager.AppletsLayout {
    id: root

    property alias url: dir.url
    property real iconSize: FolderTools.iconSizeFromTheme(Plasmoid.configuration.iconSize)
    property FolderItemDelegate hoveredItem: null
    property FolderItemDelegate pressedItem: null
    readonly property bool useListViewMode: false
    property Base.RenameEditor editor: null
    readonly property int rows: height / cellHeight
    readonly property int columns: width / cellWidth

    property Item rubberBand: null
    property var cachedRectangleSelection: null
    property bool ctrlPressed: false

    property var draggingItems: []

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
        if (root.isContainment && isRootView && scrollArea.viewportWidth > 0) {
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
        if (root.isContainment && isRootView && scrollArea.viewportHeight > 0) {
            let extraHeight = calcExtraSpacing(iconHeight, scrollArea.viewportHeight);
            return iconHeight + extraHeight;
        } else {
            return iconHeight;
        }
    }

    function rename() {
        if (gridView.currentIndex !== -1) {
            let renameAction = model.action("rename");
            if (renameAction && !renameAction.enabled) {
                return;
            }

            if (!root.editor) {
                editor = editorComponent.createObject(listener);
            }

            root.editor.targetItem = gridView.currentItem;
        }
    }

    function cancelRename() {
        if (root.editor) {
            root.editor.targetItem = null;
        }
    }

    function linkHere(sourceUrl) {
        dir.linkHere(sourceUrl);
    }

    function handleDragMove(x, y) {
        let pos = mapToItem(gridView.contentItem, x, y);
        let item = gridView.safeItemAt(pos.x, pos.y);

        if (item && item.isDir) {
            hoveredItem = item;
        } else {
            hoveredItem = null;
        }
    }

    function endDragMove() {
        if (hoveredItem && !hoveredItem.popupDialog) {
            hoveredItem = null;
        }
    }

    function dropItemAt(pos) {
        let item = gridView.safeItemAt(pos.x, pos.y);

        if (item && !item.blank) {
            gridView.currentIndex = item.index
            return positioner.map(item.index);
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

        root.cachedRectangleSelection = indices;
    }

    onCachedRectangleSelectionChanged: {
        if (cachedRectangleSelection === null) {
            return;
        }

        dir.updateSelection(cachedRectangleSelection, root.ctrlPressed);
    }

    function generateDragImage() {
        for (let i = 0; i < repeater.count; i++) {
            let item = repeater.itemAt(i) as FolderItemDelegate;
            if (item) {
                item.updateDragImage();
            }
        }
    }

    Folder.EventGenerator {
        id: eventGenerator
    }

    MouseArea {
        anchors.fill: parent
        property point pressPos
        enabled: !Plasmoid.containment.corona.editMode && !root.editMode
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        function pressCanceled() {
            if (root.rubberBand) {
                root.rubberBand.close();
                root.rubberBand = null;

                root.cachedRectangleSelection = null;
                dir.unpinSelection();
            }
        }

        onPressed: (mouse) => pressPos = Qt.point(mouse.x, mouse.y)
        onCanceled: pressCanceled()
        onReleased: pressCanceled()
        onClicked: (mouse) => {
            if (!root.isDrag(pressPos.x, pressPos.y, mouse.x, mouse.y)) {
                dir.clearSelection();
            }
        }
        onPositionChanged: (mouse) => {
            if (root.rubberBand) {
                root.rubberBand.x = Math.min(pressPos.x, mouse.x);
                root.rubberBand.y = Math.min(pressPos.y, mouse.y);
                root.rubberBand.width = Math.max(1, Math.max(pressPos.x, mouse.x) - root.rubberBand.x);
                root.rubberBand.height = Math.max(1, Math.max(pressPos.y, mouse.y) - root.rubberBand.y);
                Qt.callLater(root.rectangleSelect, rubberBand.x, rubberBand.y, rubberBand.width, rubberBand.height, root.rubberBand);
            } else if (root.isDrag(pressPos.x, pressPos.y, mouse.x, mouse.y)) {
                // TODO: Disable rubberband in popup list view mode or while renaming
                //if (main.editor && main.editor.targetItem) {
                //    return;
                //}
                dir.pinSelection();
                root.rubberBand = rubberBandObject.createObject(root, {x: pressPos.x, y: pressPos.y});
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
                    root.generateDragImage();
                    rubberBand.destroy();
                }
            }
        }
    }


    Folder.ItemViewAdapter {
        id: viewAdapter

        adapterView: root
        adapterModel: positioner
        adapterIconSize: root.iconSize * 2
        adapterVisibleArea: Qt.rect(0, 0, root.width, root.height)

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
            usedByContainment: true//root.isContainment && main.isRootView
            sortDesc: Plasmoid.configuration.sortDesc
            sortDirsFirst: Plasmoid.configuration.sortDirsFirst
            parseDesktopFiles: (Plasmoid.configuration.url === "desktop:/")
            previews: Plasmoid.configuration.previews
            previewPlugins: Plasmoid.configuration.previewPlugins
            applet: Plasmoid

           onRequestRename: root.rename()

            onPopupMenuAboutToShow: (dropJob, mimeData, x, y) => {
                if (Plasmoid.isContainment && !Plasmoid.immutable) {
                    root.processMimeData(mimeData, x, y, dropJob);
                }
            }

            // Create drag images before dragging
            // Due to async operations we can't call this before dragging starts,
            // but we have to call it after a selection is done
            onSelectionDone: root.generateDragImage()


            onMove: (x, y, urls) => {
                print("MOVE",x,y,urls)
                root.generateDragImage(); // in case the user drags again
            }
        }

        delegate: FolderItemDelegate {
            id: delegate

            isOnRootView: true

            Component.onCompleted: {
                root.positionItem(this);
            }

            width: root.cellWidth
            height: root.cellHeight

            preferredLayoutDirection: ContainmentLayoutManager.AppletsLayout.LeftToRight // TODO: all folderview layout preferences and RTL mode
        }
    }
}
