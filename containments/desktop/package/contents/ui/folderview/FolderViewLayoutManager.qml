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

import org.kde.private.desktopcontainment.folder as Folder

import org.kde.plasma.private.containmentlayoutmanager as ContainmentLayoutManager

import "../code/FolderTools.js" as FolderTools
import ".." as Base

ContainmentLayoutManager.AppletsLayout {
    id: root

    property alias url: dir.url
    property real iconSize: FolderTools.iconSizeFromTheme(Plasmoid.configuration.iconSize)
    property FolderItemDelegate hoveredItem: null
    readonly property bool useListViewMode: false
    property Base.RenameEditor editor: null
    readonly property int rows: height / cellHeight
    readonly property int columns: width / cellWidth

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
        let dropPos = mapToItem(gridView.contentItem, pos.x, pos.y);
        dir.drop(target, event, dropItemAt(dropPos), Plasmoid.isContainment && !Plasmoid.immutable);
    }

    function generateDragImage() {
        for (let i = 0; i < gridView.count; i++) {
            let item = gridView.itemAtIndex(i) as FolderItemDelegate;
            if (item) {
                item.updateDragImage();
            }
        }
    }

    Repeater {
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

           // onRequestRename: main.rename()

            onPopupMenuAboutToShow: (dropJob, mimeData, x, y) => {
                if (Plasmoid.isContainment && !Plasmoid.immutable) {
                    root.processMimeData(mimeData, x, y, dropJob);
                }
            }
/*
            // Create drag images before dragging
            // Due to async operations we can't call this before dragging starts,
            // but we have to call it after a selection is done
            onSelectionDone: main.generateDragImage()

            onListingCompleted: {
                if (!gridView.model && root.expanded) {
                    gridView.model = positioner;
                    gridView.currentIndex = inPanel ? 0 : -1;
                } else if (main.goingBack) {
                    main.goingBack = false;
                    gridView.currentIndex = Math.min(main.lastPosition.index, gridView.count - 1);
                    setSelected(positioner.map(gridView.currentIndex));
                    gridView.contentY = main.lastPosition.yPosition * gridView.contentHeight;
                }
            }*/
        }

        delegate: FolderItemDelegate {
            id: delegate

            isOnRootView: true

            Component.onCompleted: {
                root.positionItem(this);
            }

            width: root.cellWidth
            height: root.cellHeight

            preferredLayoutDirection: ContainmentLayoutManager.AppletsLayout.LeftToRight
        }
    }
}
