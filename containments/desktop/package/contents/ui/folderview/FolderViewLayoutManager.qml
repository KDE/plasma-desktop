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

    property Item rubberBand: null
    property var cachedRectangleSelection: null
    property bool ctrlPressed: false

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
        let dropPos = mapToItem(gridView.contentItem, pos.x, pos.y);
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
        id: listener

        anchors.fill: parent

        enabled: !Plasmoid.containment.corona.editMode && !root.editMode

        acceptedButtons: Qt.LeftButton
        hoverEnabled: true

        property int pressX: -1
        property int pressY: -1
        property var cPress: null

        onPressed: mouse => {
            root.forceActiveFocus();

            // Commit a possible inline rename operation
            if (root.editor && root.editor.targetItem) {
                root.editor.commit();
            }

            if (mouse.source === Qt.MouseEventSynthesizedByQt) {
                root.hoveredItem?.hideToolTip();
            }

            root.ctrlPressed = (mouse.modifiers & Qt.ControlModifier);

            pressX = mouse.x;
            pressY = mouse.y;
            cPress = mapToItem(root, mouse.x, mouse.y);

            // Pressing on the empty desktop space clears the selection unless Ctrl is held
            if ((!root.hoveredItem || root.hoveredItem.blank) && !root.ctrlPressed) {
                dir.clearSelection();
            }
        }

        onPositionChanged: mouse => {
            root.ctrlPressed = (mouse.modifiers & Qt.ControlModifier);

            // Update rubberband geometry.
            if (root.rubberBand) {
                let rB = root.rubberBand;
                let cPos = mapToItem(root, mouse.x, mouse.y);

                if (cPos.x < listener.cPress.x) {
                    rB.x = Math.max(0, cPos.x);
                    rB.width = Math.abs(rB.x - listener.cPress.x);
                } else {
                    rB.x = listener.cPress.x;
                    rB.width = Math.min(root.width - rB.x, Math.abs(rB.x - cPos.x));
                }

                if (cPos.y < listener.cPress.y) {
                    rB.y = Math.max(0, cPos.y);
                    rB.height = Math.abs(rB.y - listener.cPress.y);
                } else {
                    rB.y = listener.cPress.y;
                    rB.height = Math.min(root.height - rB.y, Math.abs(rB.y - cPos.y));
                }

                // Ensure rubberband is at least 1px in size or else it will become
                // invisible and not match any items.
                rB.width = Math.max(1, rB.width);
                rB.height = Math.max(1, rB.height);

                Qt.callLater(root.rectangleSelect, rB.x, rB.y, rB.width, rB.height, root.rubberBand);

                return;
            }

            // Drag initiation.
            if (pressX !== -1 && root.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                // Disable rubberband while renaming
                if (root.editor && root.editor.targetItem) {
                    return;
                }

                dir.pinSelection();
                root.rubberBand = rubberBandObject.createObject(root, {x: listener.cPress.x, y: listener.cPress.y});
            }
        }

        onCanceled: pressCanceled()

        onReleased: pressCanceled()

        onClicked: mouse => {
            clearPressState();

            // Only replay genuine clicks, not the release of a rubberband drag:
            // BUG 357367: replay the mouse event, so containment actions assigned
            // to the left mouse button keep working.
            if (Plasmoid.isContainment && root.containmentItem
                && !root.isDrag(listener.cPress.x, listener.cPress.y, mouse.x, mouse.y)) {
                const pos = mapToItem(root.containmentItem, mouse.x, mouse.y);
                eventGenerator.sendMouseEvent(root.containmentItem,
                    Folder.EventGenerator.MouseButtonPress,
                    pos.x, pos.y, mouse.button, mouse.buttons, mouse.modifiers);
            }
        }

        onContainsMouseChanged: {
            if (!containsMouse && !root.rubberBand) {
                clearPressState();

                if (!(root.hoveredItem?.popupDialog?.visible ?? false)) {
                    root.hoveredItem = null;
                }
            }
        }

        function pressCanceled() {
            if (root.rubberBand) {
                root.rubberBand.close();
                root.rubberBand = null;

                root.cachedRectangleSelection = null;
                dir.unpinSelection();
            }

            clearPressState();
        }

        function clearPressState() {
            pressX = -1;
            pressY = -1;
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
