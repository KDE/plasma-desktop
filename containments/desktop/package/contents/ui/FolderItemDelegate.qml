/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window
import Qt5Compat.GraphicalEffects

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.kquickcontrolsaddons

import org.kde.private.desktopcontainment.folder as Folder

Item {
    id: main

    property int index:          model.index
    property string name:        model.blank ? "" : model.display
    property string nameWrapped: model.blank ? "" : model.displayWrapped
    property bool blank:         model.blank
    property bool selected:      model.blank ? false : model.selected
    property bool isDir:           loader.item ? loader.item.isDir : false
    property bool isOnRootView: false
    property QtObject popupDialog: loader.item ? loader.item.popupDialog    : null
    property Item iconArea:        loader.item ? loader.item.iconArea       : null
    property Item label:           loader.item ? loader.item.label          : null
    property Item labelArea:       loader.item ? loader.item.labelArea      : null
    property Item actionsOverlay:  loader.item ? loader.item.actionsOverlay : null
    property Item hoverArea:       loader.item ? loader.item.hoverArea      : null
    property Item frame:           loader.item ? loader.item.frame          : null
    property Item toolTip:         loader.item ? loader.item.toolTip        : null
    property real contentHeight:   loader.item && !root.useListViewMode ? loader.item.contentHeight : null
    Accessible.name: name
    Accessible.role: Accessible.Canvas

    // This MouseArea exists to intercept press and hold; preventing edit mode
    // from being triggered when pressing and holding on an icon (if there is one).
    MouseArea {
        anchors.fill: parent
        visible: !main.blank
    }

    function openPopup() {
        if (isDir) {
            loader.item.openPopup();
        }
    }

    function closePopup() {
        if (popupDialog && popupDialog.allowClosing) {
            popupDialog.requestDestroy();
            loader.item.popupDialog = null;
        }
    }

    Loader {
        id: loader

        // On the desktop we pad our cellSize to avoid a gap at the right/bottom of the screen.
        // The padding per item is quite small and causes the delegate to be positioned on fractional pixels
        // leading to blurry rendering. The Loader is offset to account for this.
        x: -main.x % 1
        y: -main.y % 1
        width: parent.width
        height: parent.height

        visible: status === Loader.Ready

        active: !model.blank

        sourceComponent: delegateImplementation

        asynchronous: true
    }

    function updateDragImage() {
        if (selected && !blank) {
            loader.grabToImage(result => {
                dir.addItemDragImage(positioner.map(index), main.x + loader.x, main.y + loader.y, loader.width, loader.height, result.image);
            });
        }
    }

    Component {
        id: delegateImplementation

        Item {
            id: impl

            anchors.fill: parent

            property bool blank: model.blank
            property bool isDir: model.blank ? false : model.isDir
            property bool hovered: (main.GridView.view.hoveredItem === main)
            property QtObject popupDialog: null
            property Item iconArea: icon
            property Item label: label
            property Item labelArea: label
            property Item actionsOverlay: actions
            property Item hoverArea: toolTip
            property Item frame: frameLoader
            property Item toolTip: toolTip
            property Item selectionButton: selectionButtonComponent.createObject(actions)
            property Item popupButton: null
            property int contentHeight: frameLoader.height + frameLoader.y * 2

            readonly property bool iconAndLabelsShouldlookSelected: impl.hovered


            Connections {
                target: model

                function onSelectedChanged() {
                    if (dir.usedByContainment && model.selected) {
                        gridView.currentIndex = model.index;
                    }
                }
            }

            onHoveredChanged: {
                if (hovered) {
                    if (Plasmoid.configuration.selectionMarkers && Qt.styleHints.singleClickActivation) {
                        selectionButton.visible = true;
                    }

                    if (model.isDir) {
                        if (!main.GridView.view.isRootView || root.containsDrag) {
                            hoverActivateTimer.restart();
                        }

                        if (Plasmoid.configuration.popups && !root.useListViewMode) {
                            popupButton = popupButtonComponent.createObject(actions);
                        }
                    }
                } else if (!hovered) {
                    if (popupDialog != null) {
                        closePopup();
                    }

                    selectionButton.visible = false;

                    if (popupButton) {
                        popupButton.destroy();
                        popupButton = null;
                    }
                }
            }

            function openPopup() {
                if (folderViewDialogComponent.status === Component.Ready) {
                    impl.popupDialog = folderViewDialogComponent.createObject(impl);
                    impl.popupDialog.visualParent = icon;
                    impl.popupDialog.url = Folder.DesktopSchemeHelper.getDesktopUrl(model.linkDestinationUrl);
                    impl.popupDialog.visible = true;
                }
            }

            PlasmaCore.ToolTipArea {
                id: toolTip
                anchors.fill: impl

                active: (Plasmoid.configuration.toolTips || label.truncated)
                        && popupDialog === null
                        && !model.blank
                interactive: false
                location: root.useListViewMode ? (Plasmoid.location === PlasmaCore.Types.LeftEdge ? PlasmaCore.Types.LeftEdge : PlasmaCore.Types.RightEdge) : Plasmoid.location

                onContainsMouseChanged:  {
                    if (containsMouse && !model.blank) {
                        if (toolTip.active) {
                            toolTip.icon = model.decoration;
                            toolTip.mainText = model.display;

                            if (model.size !== undefined) {
                                toolTip.subText = model.type + "\n" + model.size;
                            } else {
                                toolTip.subText = model.type;
                            }
                        }

                        Qt.callLater(() => {
                            // Workaround for Qt Bug: https://bugreports.qt.io/browse/QTBUG-117444
                            // In some cases the signal order is reversed:
                            // - first it's delivered to the new object with "true" value
                            // - next it's delivered to the old object with "false" value
                            // In this case when the signal is emitted with "false" (to the old object),
                            // it's also delivered to the "FolderView" which sets the "hoveredItem" to
                            // "null" right after we set it here.
                            // The solution is to call later and check again to make sure if we still contains
                            // mouse and next set the "hoveredItem". In this approach the "FolderView" sets the
                            // old "hoveredItem" to "null" and next we set it to the new item here.
                            if (containsMouse && !model.blank) {
                                main.GridView.view.hoveredItem = main;
                            }
                        })
                    }
                }

                states: [
                    State { // icon view
                        when: !root.useListViewMode

                        AnchorChanges {
                            target: toolTip
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        PropertyChanges {
                            target: toolTip
                            y: frameLoader.y + icon.y
                            width: Math.max(icon.paintedWidth, label.paintedWidth)
                            height: (label.y + label.paintedHeight) - y
                        }
                    },
                    State { // list view
                        when: root.useListViewMode

                        AnchorChanges {
                            target: toolTip
                            anchors.horizontalCenter: undefined
                        }

                        PropertyChanges {
                            target: toolTip
                            x: frameLoader.x
                            y: frameLoader.y
                            width: frameLoader.width
                            height: frameLoader.height
                        }
                    }
                ]
            }

            Loader {
                id: frameLoader

                x: root.useListViewMode ? 0 : Kirigami.Units.smallSpacing
                y: root.useListViewMode ? 0 : Kirigami.Units.smallSpacing

                property Item iconShadow: null
                property string prefix: ""

                sourceComponent: frameComponent
                active: impl.iconAndLabelsShouldlookSelected || (model?.selected ?? false)
                asynchronous: true

                width: {
                    if (root.useListViewMode) {
                        if (main.GridView.view.overflowing) {
                            return parent.width - Kirigami.Units.smallSpacing;
                        } else {
                            return parent.width;
                        }
                    }

                    return parent.width - (Kirigami.Units.smallSpacing * 2);
                }

                height: root.useListViewMode
                                ? parent.height
                                // the smallSpacings are for padding
                                : icon.height + label.implicitHeight + (Kirigami.Units.smallSpacing * 3)

                Kirigami.Icon {
                    id: icon

                    z: 2

                    states: [
                        State { // icon view
                            when: !root.useListViewMode

                            AnchorChanges {
                                target: icon
                                anchors.top: parent.top
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        },
                        State { // list view
                            when: root.useListViewMode

                            AnchorChanges {
                                target: icon
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    ]

                    anchors {
                        topMargin: Kirigami.Units.smallSpacing
                        leftMargin: Kirigami.Units.smallSpacing
                    }

                    width: root.useListViewMode ? main.GridView.view.iconSize : (parent.width - 2 * Kirigami.Units.smallSpacing)
                    height: main.GridView.view.iconSize

                    opacity: {
                        if (root.useListViewMode && selectionButton.visible) {
                            return 0.3;
                        }

                        if (model.isHidden) {
                            return 0.6;
                        }

                        return 1.0;
                    }

                    animated: false

                    source: model.decoration
                }

                PlasmaExtras.ShadowedLabel {
                    id: label

                    readonly property bool renaming: (editor && editor.targetItem === main)

                    z: 2 // So it's always above the highlight effect

                    states: [
                        State { // icon view
                            when: !root.useListViewMode

                            AnchorChanges {
                                target: label
                                anchors.top: icon.bottom
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            PropertyChanges {
                                target: label
                                anchors.topMargin: Kirigami.Units.smallSpacing
                                width: parent.width - Kirigami.Units.smallSpacing
                                maximumLineCount: Plasmoid.configuration.textLines
                                horizontalAlignment: Text.AlignHCenter
                            }
                        },
                        State { // list view
                            when: root.useListViewMode

                            AnchorChanges {
                                target: label
                                anchors.left: icon.right
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            PropertyChanges {
                                target: label
                                anchors.leftMargin: Kirigami.Units.smallSpacing * 2
                                anchors.rightMargin: Kirigami.Units.smallSpacing * 2
                                width: parent.width - icon.width - (Kirigami.Units.smallSpacing * 4)
                                maximumLineCount: 1
                                horizontalAlignment: Text.AlignLeft
                            }
                        }
                    ]

                    color: {
                        if (main.isOnRootView) {
                            // In this situation there's a shadow or a background rect, both of which are always black
                            return "white";
                        }

                        if (model.selected) {
                            return Kirigami.Theme.highlightedTextColor;
                        }

                        return Kirigami.Theme.textColor;

                    }
                    visible: !renaming
                    renderShadow: main.isOnRootView && !renaming
                    opacity: model.isHidden ? 0.6 : 1

                    text: main.nameWrapped
                    font.italic: (model?.isLink ?? false)
                    wrapMode: (maximumLineCount === 1) ? Text.NoWrap : Text.Wrap
                    horizontalAlignment: Text.AlignHCenter
                }

                Component {
                    id: frameComponent

                    PlasmaExtras.Highlight {
                        // Workaround for a bug where the frameComponent does not
                        // get unloaded when items are dragged to a different
                        // place on the desktop.
                        visible: this === frameLoader.item
                        hovered: impl.iconAndLabelsShouldlookSelected
                        pressed: model.selected
                        active: Window.active
                    }
                }

                Component {
                    id: selectionButtonComponent

                    FolderItemActionButton {
                        element: model.selected ? "remove" : "add"

                        onClicked: {
                            dir.toggleSelected(positioner.map(index));
                            main.GridView.view.currentIndex = index;
                        }
                    }
                }

                Component {
                    id: popupButtonComponent

                    FolderItemActionButton {
                        visible: main.GridView.view.isRootView && (popupDialog == null)

                        element: "open"

                        onClicked: {
                            dir.setSelected(positioner.map(index));
                            main.GridView.view.currentIndex = index;
                            openPopup();
                        }
                    }
                }

                Component {
                    id: iconShadowComponent

                    DropShadow {
                        anchors.fill: icon

                        z: 1

                        verticalOffset: 1

                        radius: 5.0
                        samples: radius * 2 + 1
                        spread: 0.05

                        color: "black"

                        opacity: model.isHidden ? 0.3 : 0.6

                        source: icon
                    }
                }
            }

            Column {
                id: actions

                visible: {
                    if (main.GridView.view.isRootView && root.containsDrag) {
                        return false;
                    }

                    if (!main.GridView.view.isRootView && main.GridView.view.dialog && main.GridView.view.dialog.containsDrag) {
                        return false;
                    }

                    if (popupDialog) {
                        return false;
                    }

                    return true;
                }

                anchors {
                    left: frameLoader.left
                    top: frameLoader.top
                    leftMargin: root.useListViewMode ? (icon.x + (icon.width / 2)) - (width / 2) : 0
                    topMargin: root.useListViewMode ? (icon.y + (icon.height / 2)) - (height / 2) : 0
                }

                width: implicitWidth
                height: implicitHeight
            }

            Component.onCompleted: {
                selectionButton.visible = false;
                if (Plasmoid.isContainment && main.GridView.view.isRootView && root.GraphicsInfo.api === GraphicsInfo.OpenGL) {
                    frameLoader.iconShadow = iconShadowComponent.createObject(frameLoader);
                }
            }
        }
    }
}
