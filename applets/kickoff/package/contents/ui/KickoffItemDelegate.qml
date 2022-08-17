/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>
    Copyright (C) 2015-2018  Eike Hein <hein@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>
    Copyright (C) 2021 by Noah Davis <noahadvs@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
    readonly property bool textUnderIcon: display === PC3.AbstractButton.TextUnderIcon
    property bool isCategory: false
    readonly property bool hasActionList: model && (model.favoriteId !== null || ("hasActionList" in model && model.hasActionList === true))
    property var actionList: null
    property bool isSearchResult: false
    readonly property bool menuClosed: ActionMenu.menu.status == 3 // corresponds to DialogStatus.Closed

    property bool dragEnabled: enabled && !root.isCategory
        && plasmoid.immutability !== PlasmaCore.Types.SystemImmutable

    function openActionMenu(x = undefined, y = undefined) {
        if (!root.hasActionList) { return }
        // fill actionList only when needed to prevent slowness when changing app categories rapidly.
        if (root.actionList === null) {
            let allActions = model.actionList
            const favoriteActions = Tools.createFavoriteActions(
                i18n, //i18n() function callback
                view.model.favoritesModel,
                model.favoriteId)
            if (favoriteActions) {
                if (allActions && allActions.length > 0) {
                    allActions.push({ "type": "separator" });
                    allActions.push.apply(allActions, favoriteActions);
                } else {
                    allActions = favoriteActions;
                }
            }
            root.actionList = allActions
        }
        if (actionList && actionList.length > 0) {
            ActionMenu.plasmoid = plasmoid
            ActionMenu.menu.visualParent = root
            if (x !== undefined && y !== undefined) {
                ActionMenu.menu.open(x, y)
            } else {
                ActionMenu.menu.openRelative()
            }
        }
    }

    // The default Z value for delegates is 1. The default Z value for the section delegate is 2.
    // The highlight gets a value of 3 while the drag is active and then goes back to the default value of 0.
    z: Drag.active ? 4 : 1

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    // We use an increased fixed vertical padding to improve touch usability
    leftPadding: KickoffSingleton.listItemMetrics.margins.left
        + (!textUnderIcon && mirrored ? KickoffSingleton.fontMetrics.descent : 0)
    rightPadding: KickoffSingleton.listItemMetrics.margins.right
        + (!textUnderIcon && !mirrored ? KickoffSingleton.fontMetrics.descent : 0)
    topPadding: PlasmaCore.Units.smallSpacing*2
    bottomPadding: PlasmaCore.Units.smallSpacing*2

    spacing: KickoffSingleton.fontMetrics.descent

    enabled: !model.disabled
    hoverEnabled: false

    icon.width: PlasmaCore.Units.iconSizes.smallMedium
    icon.height: PlasmaCore.Units.iconSizes.smallMedium

    text: model.name ?? model.display
    Accessible.description: root.description != root.text ? root.description : ""

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
            if(view.model.trigger && view.model.trigger(index, "", null)) {
                if (plasmoid.hideOnWindowDeactivate) {
                    plasmoid.expanded = false;
                }
            }
        }
    }

    background: null
    contentItem: GridLayout {
        baselineOffset: label.y + label.baselineOffset
        columnSpacing: parent.spacing
        rowSpacing: parent.spacing
        flow: root.textUnderIcon ? GridLayout.TopToBottom : GridLayout.LeftToRight
        PlasmaCore.IconItem {
            id: iconItem
            Layout.alignment: root.textUnderIcon ? Qt.AlignHCenter | Qt.AlignBottom : Qt.AlignLeft | Qt.AlignVCenter
            implicitWidth: root.icon.width
            implicitHeight: root.icon.height
            animated: false
            usesPlasmaTheme: false
            source: root.decoration || root.icon.name || root.icon.source
        }
        PC3.Label {
            id: label
            Layout.alignment: root.textUnderIcon ? Qt.AlignHCenter | Qt.AlignTop : Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.preferredHeight: root.textUnderIcon && lineCount === 1 ? implicitHeight * 2 : implicitHeight
            text: root.text
            elide: Text.ElideRight
            horizontalAlignment: root.textUnderIcon ? Text.AlignHCenter : Text.AlignLeft
            verticalAlignment: root.textUnderIcon ? Text.AlignTop : Text.AlignVCenter
            maximumLineCount: 2
            wrapMode: Text.Wrap
            textFormat: root.model && root.model.isMultilineText ? Text.StyledText : Text.PlainText
        }
    }

    PC3.Label {
        id: descriptionLabel
        parent: root
        anchors {
            left: root.contentItem.left
            right: root.contentItem.right
            baseline: root.contentItem.baseline
            leftMargin: root.textUnderIcon ? 0 : root.implicitContentWidth + root.spacing
            baselineOffset: root.textUnderIcon ? implicitHeight : 0
        }
        visible: !textUnderIcon && text.length > 0
        enabled: false
        text: root.Accessible.description
        elide: Text.ElideRight
        horizontalAlignment: root.textUnderIcon ? Text.AlignHCenter : Text.AlignRight
        verticalAlignment: root.textUnderIcon ? Text.AlignTop : Text.AlignVCenter
        maximumLineCount: 1
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
                && root.dragEnabled && root.Drag.imageSource == ""
            ) {
                iconItem.grabToImage((result) => {
                    return root.Drag.imageSource = result.url
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
        const showDescription = descriptionLabel.truncated
            || (textUnderIcon && Accessible.description.length > 0)
        if (label.truncated && showDescription) {
            return `${text} (${description})`
        } else if (showDescription) {
            return description
        }
        return ""
    }
    PC3.ToolTip.visible: mouseArea.containsMouse && PC3.ToolTip.text.length > 0
    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
}
