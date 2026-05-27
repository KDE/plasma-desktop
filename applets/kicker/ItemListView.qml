/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.private.kicker as Kicker

PlasmaComponents3.ScrollView {
    id: itemList

    signal exited
    signal keyNavigationAtListEnd
    signal navigateLeftRequested
    signal navigateRightRequested
    signal interactionConcluded

    property Item mainSearchField: null
    property Kicker.SubMenu dialog: null
    property Kicker.SubMenu childDialog: null
    property bool iconsEnabled: false
    property bool dynamicResize : true
    property bool showDescriptionInTooltip: false
    property int innerLeftMargin: 0
    property int innerRightMargin: 0

    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem
    property alias keyNavigationWraps: listView.keyNavigationWraps
    property alias model: listView.model
    property alias count: listView.count
    property alias resetOnExitDelay: resetIndexTimer.interval


    implicitWidth: listView.implicitWidth + leftPadding + rightPadding
    implicitHeight: listView.contentHeight + topPadding + bottomPadding

    Layout.minimumWidth: Kirigami.Units.gridUnit * 14
    Layout.maximumWidth: Math.round(Layout.minimumWidth * 1.5)
    Layout.maximumHeight: contentHeight

    PlasmaComponents3.ScrollBar.horizontal.policy: PlasmaComponents3.ScrollBar.AlwaysOff

    function resetDelegateSizing() { // only needed when submenus are reused, called from ItemListDialog
        listView.maxDelegateImplicitWidth = 0
    }

    function subMenuForCurrentItem() {
        if (!kicker.expanded || !itemList.model || itemList.currentIndex === -1 || ActionMenu.opened) {
            return;
        }
        if (itemList && !(itemList.currentItem as ItemListDelegate).hasChildren) {
            clearChildDialog();
        } else if (!itemList.childDialog) {
            itemList.childDialog = itemListDialogComponent.createObject(itemList, {
                mainSearchField: mainSearchField,
                visualParent: listView.currentItem,
                model: model.modelForRow(listView.currentIndex),
                visible: Qt.binding(() => itemList.Window.window.visible),
                dialogMirrored: itemList.LayoutMirroring.enabled
            });
            itemList.childDialog.index = listView.currentIndex;
            (itemList.childDialog as ItemListDialog).interactionConcluded.connect(itemList.interactionConcluded)

            itemList.childDialog.mainItem.forceActiveFocus(Qt.TabFocusReason)
            windowSystem.forceActive(itemList.childDialog.mainItem); // only for X11; TODO Plasma 6.8: remove

        } else {
            itemList.childDialog.model = model.modelForRow(itemList.currentIndex);
            itemList.childDialog.visualParent = listView.currentItem;
            itemList.childDialog.index = listView.currentIndex;
        }
    }

    function clearChildDialog() {
        if (childDialog) {
            childDialog.delayedDestroy()
            childDialog = null
        }
    }

    Keys.priority: Keys.AfterItem
    Keys.forwardTo: [itemList.mainSearchField]

    onHoveredChanged: {
        const parentItem = itemList.dialog?.visualParent as ItemListDelegate
        if (hovered && parentItem) {
            parentItem.ListView.view.currentIndex = parentItem.index
        }
        Qt.callLater( () =>{
            if (ActionMenu.opened) {
                return
            } else if (hovered) {
                resetIndexTimer.stop();
            } else {
                resetIndexTimer.start();
            }
        })
    }

    ListView {
        id: listView

        width: itemList.availableWidth
        implicitHeight: contentHeight
        implicitWidth: itemList.Layout.minimumWidth

        anchors {
            top: parent.top
            left: parent.left
            leftMargin: itemList.innerLeftMargin
        }

        property int maxDelegateImplicitWidth: 0 // used to set implicitWidth

        Binding on implicitWidth {
            value: listView.maxDelegateImplicitWidth
            when: listView.maxDelegateImplicitWidth > 0 && itemList.dynamicResize
        }

        currentIndex: -1
        focus: true

        clip: height < contentHeight + topMargin + bottomMargin
        boundsBehavior: Flickable.StopAtBounds
        snapMode: ListView.SnapToItem
        spacing: 0
        keyNavigationEnabled: false

        Binding on cacheBuffer {
            when: itemList.dynamicResize
            value: 10000 // load more delegates, to get their width right
        }

        Accessible.name: itemList.Accessible.name
        Accessible.role: Accessible.List

        delegate: ItemListDelegate {
            width: listView.width - itemList.innerRightMargin
            anchors.left: listView.contentItem.left
            showIcons: itemList.iconsEnabled
            showDescriptionInTooltip: itemList.showDescriptionInTooltip
            dialogDefaultRight: !itemList.LayoutMirroring.enabled
            onInteractionConcluded: itemList.interactionConcluded()
            onOpenCategory: keyboardInitiated => {
                listView.currentIndex = index
                listView.openOrFocusSubmenu()
                if (keyboardInitiated) { itemList.childDialog.mainItem.currentIndex = 0; }
            }
            onContainsMouseChanged: {
                if (containsMouse && itemList.hoverEnabled && !isSeparator && !ActionMenu.opened) {
                    listView.currentIndex = index
                    itemList.forceActiveFocus()
                    dialogSpawnTimer.restart()
                } else if (listView.currentIndex === index) {
                    dialogSpawnTimer.stop()
                }
            }

            onImplicitWidthChanged: {
                listView.maxDelegateImplicitWidth = Math.max(listView.maxDelegateImplicitWidth, implicitWidth)
            }
        }

        highlight: PlasmaExtras.Highlight {
            anchors.left: listView.contentItem.left
            visible: !(listView.currentItem as ItemListDelegate)?.isSeparator
            pressed: !!((listView.currentItem as ItemListDelegate)?.iconAndLabelsShouldlookSelected)
            active: !!(listView.currentItem as ItemListDelegate)?.hovered
        }

        highlightMoveDuration: 0

        onCountChanged: {
            if (currentIndex == 0 && !itemList.mainSearchField.activeFocus) {
                currentItem?.forceActiveFocus();
            } else {
                currentIndex = -1;
            }
        }

        onCurrentIndexChanged: {
            if (currentIndex === itemList.childDialog?.index) {
                return;
            } else if (currentIndex === -1  || !(currentItem as ItemListDelegate)?.hasChildren || !kicker.expanded) {
                dialogSpawnTimer.stop();
                itemList.clearChildDialog();
            } else if (itemList.childDialog) {
                dialogSpawnTimer.restart();
            }
        }

        Connections {
            target: ActionMenu
            function onClosed() {
                resetIndexTimer.restart()
            }
        }

        function openOrFocusSubmenu() : void {
            if (itemList.childDialog === null) {
                itemList.subMenuForCurrentItem();
            } else {
                windowSystem.forceActive(itemList.childDialog.mainItem); // only for X11; TODO Plasma 6.8: remove
                itemList.childDialog.requestActivate()
                itemList.childDialog.mainItem.forceActiveFocus(Qt.TabFocusReason);
            }
        }

        function handleLeftRightArrow(event: KeyEvent) : void {
            let backArrowKey = (event.key === Qt.Key_Left && !itemList.LayoutMirroring.enabled) ||
                (event.key === Qt.Key_Right && itemList.LayoutMirroring.enabled)
            let forwardArrowKey = (event.key === Qt.Key_Right && !itemList.LayoutMirroring.enabled) ||
                (event.key === Qt.Key_Left && itemList.LayoutMirroring.enabled)

            if (backArrowKey) {
                if (itemList.dialog != null) {
                    itemList.dialog.destroy();
                } else {
                    itemList.navigateLeftRequested();
                }
            } else if (forwardArrowKey) {
                if (listView.currentItem !== null && (listView.currentItem as ItemListDelegate).hasChildren) {
                    openOrFocusSubmenu()
                    itemList.childDialog.mainItem.currentIndex = 0;
                }
            }
        }

        function handleUpDownArrow(event: KeyEvent) : void {
            let moveIndex = (event.key === Qt.Key_Up) ? listView.decrementCurrentIndex : listView.incrementCurrentIndex

            if (!listView.keyNavigationWraps && ((event.key === Qt.Key_Up && listView.currentIndex == 0) ||
                                                 (event.key === Qt.Key_Down && listView.currentIndex == listView.count - 1))) {
                itemList.keyNavigationAtListEnd();
            } else {
                moveIndex();

                if (listView.currentItem !== null) {
                    if ((listView.currentItem as ItemListDelegate).isSeparator) {
                        moveIndex();
                    }
                    listView.currentItem.forceActiveFocus(Qt.TabFocusReason);
                }
            }
        }

        Keys.onLeftPressed: event => handleLeftRightArrow(event)
        Keys.onRightPressed: event => handleLeftRightArrow(event)
        Keys.onUpPressed: event => handleUpDownArrow(event)
        Keys.onDownPressed: event => handleUpDownArrow(event)
        Keys.onEscapePressed: itemList.interactionConcluded()
        Keys.onPressed: event => {
            if (event.key !== Qt.Key_Tab && event.text !== "") {
                itemList.mainSearchField?.forceActiveFocus(Qt.ShortcutFocusReason);
            }
        }

        Timer {
            id: dialogSpawnTimer

            interval: 100
            repeat: false

            onTriggered: itemList.subMenuForCurrentItem()
        }

        Timer {
            id: resetIndexTimer

            interval: (itemList.dialog != null) ? 50 : 150
            repeat: false

            onTriggered: {
                if (itemList.focus && !(itemList.childDialog?.mainItem as ItemListView)?.hovered) {
                    itemList.currentIndex = -1;
                    itemList.exited();
                }
            }
        }

        Connections {
            target: itemList.childDialog?.mainItem ?? null
            function onHoveredChanged() {
                const childListView = itemList.childDialog?.mainItem as ItemListView
                if (childListView.hovered) {
                    resetIndexTimer.stop()
                }
            }
        }
    }
}
