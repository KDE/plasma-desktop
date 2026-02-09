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

    // can't use effectiveScrollBarWidth, it causes binding loops
    readonly property var actualScrollBarWidth: (itemList.contentHeight > itemList.height ? PlasmaComponents3.ScrollBar.vertical.width : 0)
    property Item mainSearchField: null
    property Kicker.SubMenu dialog: null
    property Kicker.SubMenu childDialog: null
    property bool iconsEnabled: false
    property bool dynamicResize : true

    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem
    property alias keyNavigationWraps: listView.keyNavigationWraps
    property alias model: listView.model
    property alias count: listView.count
    property alias resetOnExitDelay: resetIndexTimer.interval
    property alias showSeparators: listView.showSeparators


    implicitWidth: listView.implicitWidth + actualScrollBarWidth
    implicitHeight: listView.contentHeight

    Layout.minimumWidth: Kirigami.Units.gridUnit * 14
    Layout.maximumWidth: Math.round(Layout.minimumWidth * 1.5)
    Layout.maximumHeight: contentHeight

    PlasmaComponents3.ScrollBar.horizontal.policy: PlasmaComponents3.ScrollBar.AlwaysOff

    function resetDelegateSizing() { // only needed when submenus are reused, called from ItemListDialog
        listView.maxDelegateImplicitWidth = 0
    }

    function subMenuForCurrentItem(focusOnSpawn=false) {
        if (!kicker.expanded || !itemList.model || itemList.currentIndex === -1) {
            return;
        }
        if (itemList && !(itemList.currentItem as ItemListDelegate).hasChildren) {
            clearChildDialog();
        } else if (!itemList.childDialog) {
            itemList.childDialog = itemListDialogComponent.createObject(itemList, {
                mainSearchField: mainSearchField,
                visualParent: listView.currentItem,
                model: model.modelForRow(listView.currentIndex),
                visible: true,
                dialogMirrored: itemList.LayoutMirroring.enabled
            });
            itemList.childDialog.index = listView.currentIndex;
            (itemList.childDialog as ItemListDialog).interactionConcluded.connect(itemList.interactionConcluded)

            itemList.childDialog.mainItem.forceActiveFocus(Qt.TabFocusReason)
            windowSystem.forceActive(itemList.childDialog.mainItem); // only for X11; TODO Plasma 6.8: remove

            if (focusOnSpawn) {
                itemList.childDialog.mainItem.currentIndex = 0;
            }
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
        Qt.callLater( () =>{
            if (hovered) {
                resetIndexTimer.stop();
            } else if (itemList.childDialog && listView.currentIndex != itemList.childDialog?.index) {
                listView.currentIndex = childDialog.index
            } else if ((!itemList.childDialog || !itemList.dialog)
                && (!itemList.currentItem || !(itemList.currentItem as ItemListDelegate).menu.opened)) {
                resetIndexTimer.start();
            }
        })
    }

    ListView {
        id: listView

        width: itemList.availableWidth
        implicitHeight: contentHeight
        implicitWidth: itemList.Layout.minimumWidth

        property int maxDelegateImplicitWidth: 0 // used to set implicitWidth
        property bool showSeparators: !model.sorted // separators are mostly useless when sorted

        Binding on implicitWidth {
            value: listView.maxDelegateImplicitWidth
            delayed: true // only resize once all delegates are loaded
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

        function updateImplicitWidth () {
            implicitWidth = maxDelegateImplicitWidth
        }

        delegate: ItemListDelegate {
            showSeparators: listView.showSeparators
            showIcons: itemList.iconsEnabled
            dialogDefaultRight: !itemList.LayoutMirroring.enabled
            hoverEnabled: itemList.hoverEnabled
            onInteractionConcluded: itemList.interactionConcluded()
            onHoveredChanged: {
                if (hovered & !isSeparator) {
                    listView.currentIndex = index
                    itemList.forceActiveFocus()
                    dialogSpawnTimer.restart()
                } else if (listView.currentIndex === index) {
                    dialogSpawnTimer.stop()
                }
            }

            Connections {
                target: itemList.mainSearchField

                function onTextChanged() {
                    listView.maxDelegateImplicitWidth = 0
                }
            }
            onImplicitWidthChanged: {
                listView.maxDelegateImplicitWidth = Math.max(listView.maxDelegateImplicitWidth, implicitWidth)
            }
        }

        highlight: PlasmaExtras.Highlight {
            width: listView.width
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
            target: (listView.currentItem as ItemListDelegate)?.menu ?? null
            function onClosed() {
                resetIndexTimer.restart()
            }
        }

        function handleLeftRightArrowEnter(event: KeyEvent) : void {
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
            } else if (forwardArrowKey || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                if (listView.currentItem !== null && (listView.currentItem as ItemListDelegate).hasChildren) {
                    if (itemList.childDialog === null) {
                        itemList.subMenuForCurrentItem(true);
                    } else {
                        windowSystem.forceActive(itemList.childDialog.mainItem); // only for X11; TODO Plasma 6.8: remove
                        const childListView = itemList.childDialog.mainItem as ItemListView
                        childListView.forceActiveFocus(Qt.TabFocusReason);
                        childListView.currentIndex = 0;
                    }
                } else if (forwardArrowKey) {
                    itemList.navigateRightRequested();
                } else {
                    event.accepted = false;
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

        Keys.onLeftPressed: event => handleLeftRightArrowEnter(event)
        Keys.onRightPressed: event => handleLeftRightArrowEnter(event)
        Keys.onEnterPressed: event => handleLeftRightArrowEnter(event)
        Keys.onReturnPressed: event => handleLeftRightArrowEnter(event)
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
    }
}
