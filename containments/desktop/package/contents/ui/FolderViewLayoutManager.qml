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

import "code/FolderTools.js" as FolderTools

ContainmentLayoutManager.AppletsLayout {
    id: root

    property alias url: dir.url
    property real iconSize: FolderTools.iconSizeFromTheme(Plasmoid.configuration.iconSize)

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

    readonly property int rows: height / cellHeight
    readonly property int columns: width / cellWidth

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

        delegate: ContainmentLayoutManager.ItemContainer {
            id: delegate

            //layout: root
            required property bool blank
            required property bool isDir
            required property bool isHidden
            required property bool isLink
            required property bool selected
            required property int index
            required property string size
            required property string linkDestinationUrl
            required property string type
            required property string display
            required property string displayWrapped
            required property var decoration

            Component.onCompleted: {
                root.positionItem(this);
            }

            width: root.cellWidth
            height: root.cellHeight

            preferredLayoutDirection: ContainmentLayoutManager.AppletsLayout.LeftToRight
            editModeCondition:  ContainmentLayoutManager.ItemContainer.AfterMousePress

            contentItem: FolderItemDelegate {
                size: delegate.size
                index: delegate.index
                decoration: delegate.decoration
                isHidden: delegate.isHidden
                isLink: delegate.isLink
                display: delegate.display
                blank: delegate.blank
                isDir: delegate.isDir
                linkDestinationUrl: delegate.linkDestinationUrl
                selected: delegate.selected
                displayWrapped: delegate.displayWrapped
                type: delegate.type

                isOnRootView: true
            }
        }
    }
}
