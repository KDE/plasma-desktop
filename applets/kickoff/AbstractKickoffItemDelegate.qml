/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Gregor Taetzner <gregor@freenet.de>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2015-2018 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Templates as T
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PC3
import org.kde.kirigami as Kirigami
import "code/tools.js" as Tools
import org.kde.plasma.plasmoid

T.ItemDelegate {
    id: root

    enum AppNameFormat {
        NameOnly,
        GenericNameOnly,
        NameAndGenericName,
        GenericNameAndName
    }

    // model properties
    required property var model
    required property int index
    required property url url
    required property var decoration
    required property string description
    required property bool isMultilineText

    readonly property Flickable view: ListView.view ?? GridView.view
    property bool isCategoryListItem: false
    readonly property bool hasActionList: model && (model.favoriteId !== null || ("hasActionList" in model && model.hasActionList === true))
    property bool isSearchResult: false

    readonly property bool isSeparator: model && (model.isSeparator === true)
    property int separatorHeight: KickoffSingleton.lineSvg.horLineHeight + (2 * Kirigami.Units.smallSpacing)
    property int itemHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding)

    readonly property bool dragEnabled: enabled && !isCategoryListItem
        && Plasmoid.immutability !== PlasmaCore.Types.SystemImmutable

    readonly property alias mouseArea: mouseArea

    readonly property bool iconAndLabelsShouldlookSelected: down && !isCategoryListItem

    property bool labelTruncated: false
    property bool descriptionTruncated: false
    property bool descriptionVisible: true

    property Item dragIconItem: null

    down: mouseArea.pressed || dragHandler.active

    function openActionMenu(x = undefined, y = undefined) {
        if (!hasActionList) { return; }

        let actions = Array.from(model.actionList);
        const favoriteActions = Tools.createFavoriteActions(
            i18n, //i18n() function callback // qmllint disable unqualified
            root.view.model.favoritesModel,
            model.favoriteId,
        );
        if (favoriteActions) {
            if (actions && actions.length > 0) {
                actions.push({ "type": "separator" }, ...favoriteActions);
            } else {
                actions = favoriteActions;
            }
        }

        if (actions && actions.length > 0) {
            ActionMenu.plasmoid = kickoff;
            ActionMenu.menu.visualParent = root;
            ActionMenu.actionList = actions;
            if (x !== undefined && y !== undefined) {
                ActionMenu.menu.open(x, y);
            } else {
                ActionMenu.menu.openRelative();
            }
        }
    }

    // The default Z value for delegates is 1. The default Z value for the section delegate is 2.
    // The highlight gets a value of 3 while the drag is active and then goes back to the default value of 0.
    z: Drag.active ? 4 : 1

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: isSeparator ? separatorHeight : itemHeight

    spacing: KickoffSingleton.fontMetrics.descent

    enabled: !isSeparator && !model.disabled
    hoverEnabled: false

    text: model.compactNameWrapped ?? model.compactName ?? model.displayWrapped ?? model.display
    Accessible.role: Accessible.ListItem
    Accessible.description: root.description !== root.text ? root.description : ""
    Accessible.onPressAction: {
        root.forceActiveFocus() // trigger is focus guarded
        action.trigger()
    }

    // Using an action so that it can be replaced or manually triggered
    // using `model` () instead of `root.model` leads to errors about
    // `model` not having the trigger() function
    action: T.Action {
        onTriggered: {
            // Unless we're showing search results, eat the activation if we
            // don't have focus, to prevent the return/enter key from
            // inappropriately activating unfocused items
            // Also block activation while dragging.
            if ((!root.activeFocus && !root.isSearchResult) || dragHandler.active || touchDragHandler.active) {
                return;
            }
            root.view.currentIndex = root.index
            // if successfully triggered, close popup
            if (root.view.model.trigger && root.view.model.trigger(root.index, "", null)) {
                if (kickoff.hideOnWindowDeactivate) {
                    kickoff.expanded = false;
                }
            }
        }
    }

    function performDrag(handler: DragHandler): void {
        if (!handler.active) {
            kickoff.dragSource.Drag.active = false;
            kickoff.dragSource.Drag.imageSource = "";
            kickoff.dragSource.sourceItem = null;
            return;
        }
        root.dragIconItem.grabToImage(result => {
            if (!handler.active) {
                return;
            }
            kickoff.dragSource.sourceItem = root;
            kickoff.dragSource.Drag.imageSource = result.url;
            kickoff.dragSource.Drag.mimeData = {
                "text/uri-list" : [root.url]
            };
            kickoff.dragSource.Drag.active = handler.active;
        });
    }

    DragHandler {
        id: dragHandler
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad | PointerDevice.Stylus
        enabled: root.dragEnabled && root.dragIconItem !== null
        target: null // Using this Item fixes drag and drop causing delegates to reset to a 0 X position and overlapping each other.

        onActiveChanged: root.performDrag(this)
    }

    DragHandler {
        id: touchDragHandler
        acceptedDevices: PointerDevice.TouchScreen
        enabled: dragHandler.enabled
        target: null
        yAxis.enabled: false

        onActiveChanged: root.performDrag(this)
    }

    MouseArea {
        id: mouseArea
        parent: root
        anchors.fill: parent
        anchors.margins: 1
        // Flickable margins are not mirrored, so disable layout mirroring
        LayoutMirroring.enabled: false
        // Only for ListView since extending margins for GridView is hard
        anchors.leftMargin: root.view instanceof ListView ? -root.view.leftMargin : anchors.margins
        anchors.rightMargin: root.view instanceof ListView ? -root.view.rightMargin : anchors.margins
        hoverEnabled: root.view
            // When the movedWithWheel condition is broken, this ensures that
            // onEntered is called again without moving the mouse.
            && !root.view.movedWithWheel
            // Fix VerticalStackView animation causing view currentIndex
            // to change while delegates are moving under the mouse cursor
            && kickoff.fullRepresentationItem && !kickoff.fullRepresentationItem.contentItem.busy && !kickoff.fullRepresentationItem.blockingHoverFocus
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onEntered: {
            // - When the movedWithKeyboard condition is broken, we do not want to
            //   select the hovered item without moving the mouse.
            // - Don't highlight separators.
            // - Don't switch category items on hover if the setting isn't enabled
            if (root.view.movedWithKeyboard || root.isSeparator
                || (root.isCategoryListItem && !Plasmoid.configuration.switchCategoryOnHover)) {
                return
            }

            // forceActiveFocus() touches multiple items, so check for
            // activeFocus first to be more efficient. Keep activeFocus
            // stable if it's on the searchField to avoid unintentional
            // activation with Space key presses.
            if (!root.activeFocus && !kickoff.searchField?.activeFocus) {
                root.forceActiveFocus(Qt.MouseFocusReason)
            }
            // No need to check currentIndex first because it's
            // built into QQuickListView::setCurrentIndex() already
            root.view.currentIndex = root.index
        }
        onPressed: mouse => {
            // Select and focus on press to improve responsiveness and touch feedback
            root.view.currentIndex = root.index
            root.forceActiveFocus(Qt.MouseFocusReason)

            // We normally try to open right click menus on press like Qt Widgets
            if (mouse.button === Qt.RightButton) {
                root.openActionMenu(mouseX, mouseY)
            }
        }
        onClicked: mouse => {
            if (mouse.button === Qt.LeftButton) {
                root.action.trigger()
            }
        }

        TapHandler {
            acceptedDevices: PointerDevice.TouchScreen
            onLongPressed: root.openActionMenu(point.position.x, point.position.y)
        }
    }

    PC3.ToolTip.text: {
        if (root.labelTruncated) {
            return model.display
        } else if (root.descriptionTruncated || (!root.descriptionVisible && (root.isSearchResult
                                                                              || Plasmoid.configuration.appNameFormat > 1))) {
            return description
        }
        return ""
    }
    PC3.ToolTip.visible: mouseArea.containsMouse && PC3.ToolTip.text.length > 0
    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay

    background: null
}
