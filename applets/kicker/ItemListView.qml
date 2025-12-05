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
import org.kde.ksvg as KSvg

PlasmaComponents3.ScrollView {
    id: itemList

    signal exited
    signal keyNavigationAtListEnd
    signal navigateLeftRequested
    signal navigateRightRequested

    // can't use effectiveScrollBarWidth, it causes binding loops
    readonly property var actualScrollBarWidth: (itemList.contentHeight > itemList.height ? PlasmaComponents3.ScrollBar.vertical.width : 0)
    property Item mainSearchField: null
    property QtObject dialog: null
    property QtObject childDialog: null
    property bool iconsEnabled: false

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
        if (itemList && !itemList.currentItem.hasChildren) {
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

            windowSystem.forceActive(itemList.childDialog.mainItem); // only for X11; TODO Plasma 6.8: remove
            itemList.childDialog.mainItem.focus = true;

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
        if (hovered) {
            resetIndexTimer.stop();
        } else if (itemList.childDialog && listView.currentIndex != itemList.childDialog?.index) {
            listView.currentIndex = childDialog.index
        } else if ((!itemList.childDialog || !itemList.dialog)
            && (!itemList.currentItem || !itemList.currentItem.menu.opened)) {
            resetIndexTimer.start();
        }
    }

    ListView {
        id: listView

        width: listView.availableWidth
        implicitHeight: contentHeight
        implicitWidth: itemList.Layout.minimumWidth

        property int maxDelegateImplicitWidth: 0 // used to set implicitWidth
        property bool showSeparators: !model.sorted // separators are mostly useless when sorted

        Binding on implicitWidth {
            value: listView.maxDelegateImplicitWidth
            delayed: true // only resize once all delegates are loaded
            when: listView.maxDelegateImplicitWidth > 0
        }

        currentIndex: -1
        focus: true

        clip: height < contentHeight + topMargin + bottomMargin
        boundsBehavior: Flickable.StopAtBounds
        snapMode: ListView.SnapToItem
        spacing: 0
        keyNavigationEnabled: false
        cacheBuffer: 10000 // try to load all delegates for sizing; krunner won't return too many anyway

        function updateImplicitWidth () {
            implicitWidth = maxDelegateImplicitWidth
        }

        delegate: ItemListDelegate {
            showSeparators: listView.showSeparators
            dialogDefaultRight: !itemList.LayoutMirroring.enabled
            hoverEnabled: itemList.hoverEnabled
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
            visible: !listView.currentItem || !listView.currentItem.isSeparator
            pressed: listView.currentItem && listView.currentItem.pressed && !listView.currentItem.hasChildren
            active: listView.currentItem && listView.currentItem.hovered
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
            if (currentIndex === childDialog?.index) {
                return;
            } else if (currentIndex === -1  || !currentItem.hasChildren || !kicker.expanded) {
                dialogSpawnTimer.stop();
                itemList.clearChildDialog();
            } else if (itemList.childDialog) {
                dialogSpawnTimer.restart();
            }
        }

        onCurrentItemChanged: {
            if (currentItem) {
                currentItem.menu.closed.connect(resetIndexTimer.restart);
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
                        itemList.childDialog.mainItem.forceActiveFocus(Qt.TabFocusReason);
                        itemList.childDialog.mainItem.currentIndex = 0;
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
        Keys.onEscapePressed: kicker.expanded = false;
        Keys.onPressed: event => {
            if (event.key !== Qt.Key_Tab && event.text !== "") {
                itemList.mainSearchField?.forceActiveFocus(Qt.ShortcutFocusReason);
            }
        }

        Timer {
            id: dialogSpawnTimer

            interval: 100
            repeat: false

            onTriggered: subMenuForCurrentItem()
        }

        Timer {
            id: resetIndexTimer

            interval: (itemList.dialog != null) ? 50 : 150
            repeat: false

            onTriggered: {
                if (itemList.focus && (!itemList.childDialog || !itemList.childDialog.mainItem.hovered)) {
                    itemList.currentIndex = -1;
                    itemList.exited();
                }
            }
        }
    }
}
