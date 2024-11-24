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
import QtQuick.Layouts
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

    down: mouseArea.pressed

    function openActionMenu(x = undefined, y = undefined) {
        if (!hasActionList) { return; }

        let actions = Array.from(model.actionList);
        const favoriteActions = Tools.createFavoriteActions(
            i18n, //i18n() function callback
            view.model.favoritesModel,
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
        Accessible.name: root.text // https://bugreports.qt.io/browse/QTBUG-130360
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
                if (kickoff.hideOnWindowDeactivate) {
                    kickoff.expanded = false;
                }
            }
        }
    }

    Drag.active: mouseArea.drag.active
    Drag.dragType: Drag.Automatic
    Drag.mimeData: { "text/uri-list" : [root.url] }
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
        hoverEnabled: root.view
            // When the movedWithWheel condition is broken, this ensures that
            // onEntered is called again without moving the mouse.
            && !root.view.movedWithWheel
            // Fix VerticalStackView animation causing view currentIndex
            // to change while delegates are moving under the mouse cursor
            && kickoff.fullRepresentationItem && !kickoff.fullRepresentationItem.contentItem.busy && !kickoff.fullRepresentationItem.blockingHoverFocus
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        drag {
            axis: Drag.XAndYAxis
            target: root.dragEnabled && mouseArea.dragEnabled ? dragItem : undefined
        }
        // Using this Item fixes drag and drop causing delegates
        // to reset to a 0 X position and overlapping each other.
        Item { id: dragItem }

        onEntered: {
            // When the movedWithKeyboard condition is broken, we do not want to
            // select the hovered item without moving the mouse.
            if (root.view.movedWithKeyboard) {
                return
            }
            // Don't highlight separators.
            if (root.isSeparator) {
                return;
            }

            // forceActiveFocus() touches multiple items, so check for
            // activeFocus first to be more efficient.
            if (!root.activeFocus) {
                root.forceActiveFocus(Qt.MouseFocusReason)
            }
            // No need to check currentIndex first because it's
            // built into QQuickListView::setCurrentIndex() already
            root.view.currentIndex = index
        }
        onPressed: mouse => {
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
        onClicked: mouse => {
            if (mouse.button === Qt.LeftButton) {
                root.action.trigger()
            }
        }
        // MouseEvents for pressAndHold use Qt.MouseEventSynthesizedByQt for mouse.source,
        // which makes checking mouse.source for whether or not touch input is used useless.
        onPressAndHold: mouse => {
            if (mouse.button === Qt.LeftButton) {
                root.openActionMenu(mouseX, mouseY)
            }
        }
    }

    PC3.ToolTip.text: {
        if (root.labelTruncated && root.descriptionTruncated) {
            return model.display
        } else if (root.descriptionTruncated || !root.descriptionVisible) {
            return description
        }
        return ""
    }
    PC3.ToolTip.visible: mouseArea.containsMouse && PC3.ToolTip.text.length > 0
    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay

    background: null
}
