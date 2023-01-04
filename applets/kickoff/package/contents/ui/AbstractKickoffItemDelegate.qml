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
import QtQuick 2.15
import QtQml 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.16 as Kirigami
import "code/tools.js" as Tools

T.ItemDelegate {
    id: root

    // model properties
    required property var model
    required property int index
    required property url url
    required property var decoration
    required property string description

    readonly property Flickable view: ListView.view ?? GridView.view
    property bool isCategoryListItem: false
    readonly property bool hasActionList: model && (model.favoriteId !== null || ("hasActionList" in model && model.hasActionList === true))
    property var actionList: null
    property bool isSearchResult: false

    readonly property bool isSeparator: model && (model.isSeparator === true)
    property int separatorHeight: KickoffSingleton.lineSvg.horLineHeight + (2 * PlasmaCore.Units.smallSpacing)
    property int itemHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding)

    readonly property bool dragEnabled: enabled && !isCategoryListItem
        && plasmoid.immutability !== PlasmaCore.Types.SystemImmutable

    property bool labelTruncated: false
    property bool descriptionTruncated: false
    property bool descriptionVisible: true

    property Item dragIconItem: null

    function openActionMenu(x = undefined, y = undefined) {
        if (!hasActionList) { return; }
        // fill actionList only when needed to prevent slowness when changing app categories rapidly.
        if (actionList === null) {
            let allActions = model.actionList;
            const favoriteActions = Tools.createFavoriteActions(
                i18n, //i18n() function callback
                view.model.favoritesModel,
                model.favoriteId,
            );
            if (favoriteActions) {
                if (allActions && allActions.length > 0) {
                    allActions.push({ "type": "separator" }, ...favoriteActions);
                } else {
                    allActions = favoriteActions;
                }
            }
            actionList = allActions;
        }
        if (actionList && actionList.length > 0) {
            ActionMenu.plasmoid = plasmoid;
            ActionMenu.menu.visualParent = root;
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

    text: model.name ?? model.display
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
            if (!root.activeFocus && !root.isSearchResult) {
                return;
            }
            view.currentIndex = index
            // if successfully triggered, close popup
            if (view.model.trigger && view.model.trigger(index, "", null)) {
                if (plasmoid.hideOnWindowDeactivate) {
                    plasmoid.expanded = false;
                }
            }
        }
    }

    Drag.active: mouseArea.drag.active
    Drag.dragType: Drag.Automatic
    Drag.mimeData: { "text/uri-list" : root.url }
    Drag.onDragFinished: Drag.imageSource = ""

    MouseArea {
        id: mouseArea
        property bool dragEnabled: false
        parent: root
        anchors.fill: parent
        anchors.margins: 1
        // Flickable margins are not mirrored, so disable layout mirroring
        LayoutMirroring.enabled: false
        // Only for ListView since extending margins for GridView is hard
        anchors.leftMargin: root.view instanceof ListView ? -root.view.leftMargin : anchors.margins
        anchors.rightMargin: root.view instanceof ListView ? -root.view.rightMargin : anchors.margins
        hoverEnabled: root.view && !root.view.movedWithKeyboard
            // Fix VerticalStackView animation causing view currentIndex
            // to change while delegates are moving under the mouse cursor
            && plasmoid.fullRepresentationItem && !plasmoid.fullRepresentationItem.contentItem.busy
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        drag {
            axis: Drag.XAndYAxis
            target: root.dragEnabled && mouseArea.dragEnabled ? dragItem : undefined
        }
        // Using this Item fixes drag and drop causing delegates
        // to reset to a 0 X position and overlapping each other.
        Item { id: dragItem }
        // Using onPositionChanged instead of onEntered because it enables several
        // desirable behaviors:
        // 1. prevents changing selection while scrolling with the mouse wheel
        // 2. Prevents the cursor position resetting selection when navigating using the keyboard
        // See Bugs 455674 and 454349
        onPositionChanged: {
            // forceActiveFocus() touches multiple items, so check for
            // activeFocus first to be more efficient.
            if (!root.activeFocus) {
                root.forceActiveFocus(Qt.MouseFocusReason)
            }
            // No need to check currentIndex first because it's
            // built into QQuickListView::setCurrentIndex() already
            root.view.currentIndex = index
        }
        onPressed: {
            // Select and focus on press to improve responsiveness and touch feedback
            view.currentIndex = index
            root.forceActiveFocus(Qt.MouseFocusReason)

            // Only enable drag and drop with a mouse.
            // We don't have a good way to handle it and drag scrolling with touch.
            mouseArea.dragEnabled = mouse.source === Qt.MouseEventNotSynthesized

            // We normally try to open right click menus on press like Qt Widgets
            if (mouse.button === Qt.RightButton) {
                root.openActionMenu(mouseX, mouseY)
            } else if (mouseArea.dragEnabled && mouse.button === Qt.LeftButton
                && root.dragEnabled && root.dragIconItem && root.Drag.imageSource.toString() === ""
            ) {
                root.dragIconItem.grabToImage(result => {
                    root.Drag.imageSource = result.url
                })
            }
        }
        onClicked: if (mouse.button === Qt.LeftButton) {
            root.action.trigger()
        }
        // MouseEvents for pressAndHold use Qt.MouseEventSynthesizedByQt for mouse.source,
        // which makes checking mouse.source for whether or not touch input is used useless.
        onPressAndHold: if (mouse.button === Qt.LeftButton) {
            root.openActionMenu(mouseX, mouseY)
        }
    }

    PC3.ToolTip.text: {
        if (root.labelTruncated && root.descriptionTruncated) {
            return `${text} (${description})`
        } else if (root.descriptionTruncated || !root.descriptionVisible) {
            return description
        }
        return ""
    }
    PC3.ToolTip.visible: mouseArea.containsMouse && PC3.ToolTip.text.length > 0
    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay

    background: null
}
