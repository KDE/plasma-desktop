/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.kquickcontrolsaddons as KQuickControlsAddons
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.ksvg as KSvg

FocusScope {
    id: itemList

    property real minimumWidth: Kirigami.Units.gridUnit * 14
    property real maximumWidth: Math.round(minimumWidth * 1.5)

    height: implicitHeight
    width: implicitWidth
    implicitWidth: Math.min(Math.max(minimumWidth, scrollView.implicitWidth), maximumWidth)
    implicitHeight: listView.contentHeight

    signal exited
    signal keyNavigationAtListEnd
    signal navigateLeftRequested
    signal navigateRightRequested

    property Item focusParent: null
    property Item mainSearchField: null
    property QtObject dialog: null
    property QtObject childDialog: null
    property bool iconsEnabled: false
    property int itemHeight: Math.ceil((Math.max(Kirigami.Units.iconSizes.sizeForLabels, Kirigami.Units.iconSizes.small)
        + Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
        listItemSvg.margins.top + listItemSvg.margins.bottom)) / 2) * 2
    property int separatorHeight: showSeparators ? lineMetrics.elementRect.height + (2 * Kirigami.Units.smallSpacing) : 0

    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem
    property alias keyNavigationWraps: listView.keyNavigationWraps
    property alias model: listView.model
    property alias count: listView.count
    property alias containsMouse: listener.containsMouse
    property alias resetOnExitDelay: resetIndexTimer.interval
    property alias showSeparators: listView.showSeparators
    property alias hoverEnabled: listView.hoverEnabled

    property KSvg.SvgItem lineMetrics: KSvg.SvgItem {
        imagePath: "widgets/line"
        elementId: "horizontal-line"
    }

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
            // Gets reenabled after the dialog spawn causes a focus-in on the dialog window.
            kicker.hideOnWindowDeactivate = false;

            itemList.childDialog = itemListDialogComponent.createObject(itemList, {
                mainSearchField: mainSearchField,
                focusParent: itemList,
                visualParent: listView.currentItem,
                model: model.modelForRow(listView.currentIndex),
                visible: true,
                dialogMirrored: itemList.LayoutMirroring.enabled
            });
            itemList.childDialog.index = listView.currentIndex;

            windowSystem.forceActive(itemList.childDialog.mainItem);
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
            if (itemList.focus && (!itemList.childDialog || !itemList.childDialog.mainItem.containsMouse)) {
                itemList.currentIndex = -1;
                itemList.exited();
            }
        }
    }

    Keys.priority: Keys.AfterItem
    Keys.forwardTo: [itemList.mainSearchField]

    KQuickControlsAddons.MouseEventListener {
        id: listener

        anchors.fill: parent

        hoverEnabled: true

        onContainsMouseChanged: containsMouseChanged => {
            if (containsMouse) {
                resetIndexTimer.stop();
                itemList.forceActiveFocus();
            } else if (itemList.childDialog && listView.currentIndex != itemList.childDialog?.index) {
                listView.currentIndex = childDialog.index
            } else if ((!itemList.childDialog || !itemList.dialog)
                && (!itemList.currentItem || !itemList.currentItem.menu.opened)) {
                resetIndexTimer.start();
            }
        }

        PlasmaComponents3.ScrollView {
            id: scrollView
            anchors.fill: parent
            // can't use effectiveScrollBarWidth, it causes a binding loop if a submenu needs to be repositioned
            implicitWidth: listView.implicitWidth + (contentHeight > height ? PlasmaComponents3.ScrollBar.vertical.width : 0)

            focus: true

            ListView {
                id: listView

                implicitWidth: itemList.minimumWidth

                property int maxDelegateImplicitWidth: 0 // used to set implicitWidth
                property bool mouseMoved: true // child dialogs can activate immediately
                property bool showSeparators: !model.sorted // separators are mostly useless when sorted
                property bool hoverEnabled: true

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
                    hoverEnabled: listView.hoverEnabled
                    onHoveredChanged: {
                        if (hovered & !isSeparator) {
                            listView.currentIndex = index
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

                Connections {
                    target: searchField

                    function onTextChanged() {
                        listView.maxDelegateImplicitWidth = 0
                    }
                }
            }

            Keys.onPressed: event => {
                let backArrowKey = (event.key === Qt.Key_Left && !itemList.LayoutMirroring.enabled) ||
                    (event.key === Qt.Key_Right && itemList.LayoutMirroring.enabled)
                let forwardArrowKey = (event.key === Qt.Key_Right && !itemList.LayoutMirroring.enabled) ||
                    (event.key === Qt.Key_Left && itemList.LayoutMirroring.enabled)
                if (listView.currentItem !== null && listView.currentItem.hasChildren &&
                    (forwardArrowKey || event.key === Qt.Key_Return || event.key === Qt.Key_Enter)) {
                    if (itemList.childDialog === null) {
                        subMenuForCurrentItem(true)
                    } else {
                        windowSystem.forceActive(itemList.childDialog.mainItem);
                        itemList.childDialog.mainItem.focus = true;
                        itemList.childDialog.mainItem.currentIndex = 0;
                    }
                } else if (event.key === Qt.Key_Up) {
                    event.accepted = true;

                    if (!listView.keyNavigationWraps && listView.currentIndex == 0) {
                        itemList.keyNavigationAtListEnd();

                        return;
                    }

                    listView.decrementCurrentIndex();

                    if (listView.currentItem !== null) {
                        if (listView.currentItem.isSeparator) {
                            listView.decrementCurrentIndex();
                        }
                        listView.currentItem.forceActiveFocus();
                    }
                } else if (event.key === Qt.Key_Down) {
                    event.accepted = true;

                    if (!listView.keyNavigationWraps && listView.currentIndex == listView.count - 1) {
                        itemList.keyNavigationAtListEnd();

                        return;
                    }

                    listView.incrementCurrentIndex();

                    if (listView.currentItem !== null) {
                        if (listView.currentItem.isSeparator) {
                            listView.incrementCurrentIndex();
                        }
                        listView.currentItem.forceActiveFocus();
                    }

                } else if (backArrowKey && itemList.dialog != null) {
                    itemList.dialog.destroy();
                } else if (event.key === Qt.Key_Escape) {
                    kicker.expanded = false;
                } else if (event.key === Qt.Key_Tab) {
                    //do nothing, and skip appending text
                } else if (event.text !== "") {
                    if (itemList.mainSearchField) {
                        itemList.mainSearchField.forceActiveFocus();
                    }
                } else if (backArrowKey) {
                    itemList.navigateLeftRequested();
                } else if (forwardArrowKey) {
                    itemList.navigateRightRequested();
                }
            }

        }
    }

    Component.onCompleted: {
        windowSystem.monitorWindowFocus(itemList);
    }
}
